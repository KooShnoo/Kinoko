#include "host/System.hh"
#include <Logger.hh>
#include <filesystem>
#include <fstream>
#include <game/system/GhostFile.hh>
#include <print>
#include <vector>

#if defined(__arm64__) || defined(__aarch64__)
static void FlushDenormalsToZero() {
    uint64_t fpcr;
    asm("mrs %0,   fpcr" : "=r"(fpcr));
    asm("msr fpcr, %0" ::"r"(fpcr | (1 << 24)));
}
#elif defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <immintrin.h>

static void FlushDenormalsToZero() {
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
}
#endif

// int main(int argc, char **argv) {
int main() {
    FlushDenormalsToZero();
    // return Host::KSystem::Instance().main(argc, argv);


    //     auto file = std::ifstream("samples/rmc-rta-1-29-945.rkg", std::ios::binary);
    // ASSERT(file.is_open());
    // file.seekg(0, std::ios::end);
    // std::streampos fileSize = file.tellg();
    // file.seekg(0, std::ios::beg);

    // auto rkgBuf = new u8[fileSize];
    // file.read((char *)rkgBuf, fileSize);
    // file.close();

    // auto ghost = System::GhostFile(rkgBuf);

    // auto t = ghost.m_raceTime;
    // std::println("time {}:{}.{}, est frames {}", t.min, t.sec, t.mil, t.min * 60 * 60 + t.sec * 60 + 412 + (size_t)((t.mil / 1000.0f) * 60));

    for (const auto &path : std::filesystem::directory_iterator("rkgs")) {
        auto fileSize = path.file_size();
        auto file = std::ifstream(path.path());
        
        auto rkgBuf = new u8[fileSize];
        file.read((char *)rkgBuf, fileSize);
        file.close();

        auto ghost = System::GhostFile(rkgBuf);

        std::println("time {}:{}.{}", ghost.m_raceTime.min, ghost.m_raceTime.sec, ghost.m_raceTime.mil);
    }

}