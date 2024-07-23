#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <Common.hh>

#include <game/system/KPadController.hh>

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

PlayerTransform* calc();

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

static PyObject* pilz_init(PyObject */* self */, PyObject */* args */) {
    Pilz::init();
    Py_RETURN_NONE;
}

static PyObject* pilz_calc(PyObject */* self */, PyObject */* args */) {
    // NOTE: add this in the calling python code to get the actual desired values,
    // i.e. player transform matrix and view-camera transform matrix
    const auto transform = Pilz::calc();
    const auto posrot = PyTuple_New(2);
    const auto pos = PyTuple_New(3);
    const auto rot = PyTuple_New(4);
    for (size_t i = 0; i < 3; i++) {
        PyTuple_SET_ITEM(pos, i, PyFloat_FromDouble(transform->pos[i]));
        PyTuple_SET_ITEM(rot, i, PyFloat_FromDouble(transform->rot[i]));
    }
    PyTuple_SET_ITEM(rot, 3, PyFloat_FromDouble(transform->rot[3]));

    PyTuple_SET_ITEM(posrot, 0, pos);
    PyTuple_SET_ITEM(posrot, 1, rot);


    return posrot;
}

static PyMethodDef PilzMethods[] = {
    {"provide_files", pilz_provide_files, METH_VARARGS, NULL},
    {"set_buttons", pilz_set_buttons, METH_VARARGS, NULL},
    {"set_course", pilz_set_course, METH_VARARGS, NULL},
    {"init", pilz_init, METH_VARARGS, NULL},
    {"calc", pilz_calc, METH_VARARGS, NULL},
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
