#include <cstdio>
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <condition_variable>
#include <mutex>
#include <thread>
#include <tupleobject.h>

#include <egg/math/Matrix.hh>
#include <game/system/KPadController.hh>
#include "game/kart/KartObjectManager.hh"

#include <cstddef>
#include <fstream>
#include <print>

namespace Pilz {

struct PlayerTransform {
    s32 timer;
    std::array<f32, 3> pos;
    std::array<f32, 4> rot;
};

void init();
void setButtons(u16 buttons, f32 stickX, f32 stickY, System::Trick trick);
void setCourse(Course course);
void calc();

}

void setFiles(u8* _common, size_t _commonLen, u8* _course, size_t _courseLen);

std::pair<u8*, size_t> loadFile(char * filepath) {
    auto file = std::ifstream(filepath, std::ios::binary);
    if (!file) {
        K_PANIC("File with provided path %s was not loaded correctly!", filepath);
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    u8 *buffer = new u8[size];
    file.read(reinterpret_cast<char *>(buffer), size);

    return std::pair(buffer, size);
}

static PyObject* pilz_print(PyObject */* self */, PyObject */* args */) {
    return PyUnicode_FromString("大家好!");
}

static PyObject* pilz_provide_files(PyObject */* self */, PyObject *args) {
    char* commonFilepath;
    char* courseFilepath;
    if (!PyArg_ParseTuple(args, "ss", &commonFilepath, &courseFilepath)) {
        return NULL;
    }

    auto [cmn, cmnLen] = loadFile(commonFilepath);
    auto [course, courseLen] = loadFile(courseFilepath);
    setFiles(cmn, cmnLen, course, courseLen);
    Py_RETURN_NONE;
}

static PyObject* pilz_set_buttons(PyObject */* self */, PyObject *args) {
    u16 buttons;
    f32 stickX;
    if (!PyArg_ParseTuple(args, "Hf", &buttons, &stickX)) {
        return NULL;
    }
    Pilz::setButtons(buttons, stickX, 0, System::Trick::None);
    Py_RETURN_NONE;
}

static PyObject* pilz_set_course(PyObject */* self */, PyObject *args) {
    Course course;
    if (!PyArg_ParseTuple(args, "i", &course)) {
        return NULL;
    }
    Pilz::setCourse(course);
    Py_RETURN_NONE;
}

static std::mutex pilz_mutex;
static std::condition_variable pilz_cv;
static bool pilz_paused = true;
// terribly named veriable, cry about it mwahahhahaha
static bool pilz_thread = false;

constexpr int targetFPS = 60;
constexpr std::chrono::milliseconds frameTime(1000 / targetFPS);

static PyObject* pilz_init(PyObject */* self */, PyObject */* args */) {
    Pilz::init();
    for (size_t i = 0; i < 500; ++i) {
        Pilz::calc();
    }
    if (!pilz_thread) {
        std::thread([]{
            while (true) {
                auto startTime = std::chrono::high_resolution_clock::now();
                std::unique_lock<std::mutex> lock(pilz_mutex);
                pilz_cv.wait(lock, [] { return !pilz_paused; });

                Pilz::calc();

                lock.unlock();
                auto endTime = std::chrono::high_resolution_clock::now();
                auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
                auto sleepTime = frameTime - elapsedTime;
                if (sleepTime > std::chrono::milliseconds(0)) {
                    std::this_thread::sleep_for(sleepTime);
                }
            }
        }).detach();
        pilz_thread = true;
    }
    Py_RETURN_NONE;
}

static PyObject* pilz_start(PyObject */* self */, PyObject */* args */) {
    std::unique_lock<std::mutex> lock(pilz_mutex);
    pilz_paused = false;
    lock.unlock();
    pilz_cv.notify_one();
    Py_RETURN_NONE;
}

static PyObject* pilz_stop(PyObject */* self */, PyObject */* args */) {
    std::unique_lock<std::mutex> lock(pilz_mutex);
    pilz_paused = true;
    lock.unlock();
    pilz_cv.notify_one();
    Py_RETURN_NONE;
}

static PyObject* pilz_read_state(PyObject */* self */, PyObject */* args */) {
    // const auto transform = Pilz::getPlayerTransform();
    const auto player = Kart::KartObjectManager::Instance()->object(0);
    const auto &pos = player->pos();
    const auto &rotQ = player->mainRot();
    auto rot = EGG::Matrix34f();
    rot.makeQ(rotQ);
    // auto rot = quatToEuler(mainRot);
    // const auto &pose = player->pose(); 

    // swizzled xzy
    const auto posPy = PyTuple_New(3);
    PyTuple_SET_ITEM(posPy, 0, PyFloat_FromDouble(pos.x));
    PyTuple_SET_ITEM(posPy, 1, PyFloat_FromDouble(pos.z));
    PyTuple_SET_ITEM(posPy, 2, PyFloat_FromDouble(pos.y));


    const auto rot0 = PyTuple_New(4);
    for (size_t i = 0; i < 4; ++i) {
        PyTuple_SET_ITEM(rot0, i, PyFloat_FromDouble(rot[0, i]));
    }
    const auto rot1 = PyTuple_New(4);
    for (size_t i = 0; i < 4; ++i) {
        PyTuple_SET_ITEM(rot1, i, PyFloat_FromDouble(rot[2, i]));
    }
    const auto rot2 = PyTuple_New(4);
    for (size_t i = 0; i < 4; ++i) {
        PyTuple_SET_ITEM(rot2, i, PyFloat_FromDouble(rot[1, i]));
    }

    const auto rot3 = PyTuple_New(4);
    PyTuple_SET_ITEM(rot3, 0, PyFloat_FromDouble(0));
    PyTuple_SET_ITEM(rot3, 1, PyFloat_FromDouble(0));
    PyTuple_SET_ITEM(rot3, 2, PyFloat_FromDouble(0));
    PyTuple_SET_ITEM(rot3, 3, PyFloat_FromDouble(1));
    const auto rotPy = PyTuple_New(4);
    PyTuple_SET_ITEM(rotPy, 0, rot0);
    PyTuple_SET_ITEM(rotPy, 1, rot1);
    PyTuple_SET_ITEM(rotPy, 2, rot2);
    PyTuple_SET_ITEM(rotPy, 3, rot3);


    const auto posrot = PyTuple_New(2);
    PyTuple_SET_ITEM(posrot, 0, posPy);
    PyTuple_SET_ITEM(posrot, 1, rotPy);


    return posrot;
}

static PyMethodDef PilzMethods[] = {
    {"provide_files", pilz_provide_files, METH_VARARGS, NULL},
    {"set_buttons", pilz_set_buttons, METH_VARARGS, NULL},
    {"set_course", pilz_set_course, METH_VARARGS, NULL},
    {"init", pilz_init, METH_VARARGS, NULL},
    {"start", pilz_start, METH_VARARGS, NULL},
    {"stop", pilz_stop, METH_VARARGS, NULL},
    {"read_state", pilz_read_state, METH_VARARGS, NULL},
    {"print", pilz_print, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef pilzmodule = {
    PyModuleDef_HEAD_INIT,
    "pilz",
    NULL, -1,
    PilzMethods,
    NULL, NULL, 0, NULL
};

PyMODINIT_FUNC
PyInit_pilz(void) {
    return PyModule_Create(&pilzmodule);
}
