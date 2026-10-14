#include "dsp/VocalChain.h"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

void check(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
float energy(const std::vector<float>& x, size_t begin = 0) {
    double sum=0; for(size_t i=begin;i<x.size();++i) sum += x[i]*x[i];
    return static_cast<float>(sum / static_cast<double>(x.size()-begin));
}
std::vector<float> render(d2nz::Settings settings, double rate, double frequency, float amplitude, int block=128) {
    d2nz::VocalChain chain; chain.prepare(rate); chain.setSettings(settings);
    std::vector<float> audio(static_cast<size_t>(rate));
    for(size_t i=0;i<audio.size();++i) audio[i]=amplitude*static_cast<float>(std::sin(6.283185307179586*frequency*static_cast<double>(i)/rate));
    for(size_t i=0;i<audio.size();i+=static_cast<size_t>(block)) {
        float* p=audio.data()+i; chain.process(&p,1,std::min(block,static_cast<int>(audio.size()-i)));
    }
    for(float x:audio) check(std::isfinite(x), "non-finite output");
    return audio;
}
int main() {
    try {
        int checks = 0;
        for(double rate : {44100.,48000.,96000.}) {
            auto dry=render({},rate,440,0.8f);
            check(std::abs(energy(dry)-0.32f)<0.001f,"neutral chain is not transparent"); ++checks;
            d2nz::Settings s; s.outputDb=-6;
            auto quiet=render(s,rate,440,0.8f);
            check(std::abs(energy(quiet,static_cast<size_t>(rate/2))/energy(dry,static_cast<size_t>(rate/2))-0.2511886f)<0.001f,"output gain incorrect"); ++checks;
            s={};s.clean=1;
            check(energy(render(s,rate,30,0.5f),static_cast<size_t>(rate/2))<0.2f*energy(render({},rate,30,0.5f)),"cleanup must attenuate lows"); ++checks;
            s={};s.power=1;
            check(energy(render(s,rate,440,0.8f),static_cast<size_t>(rate/2))<0.1f*energy(dry),"compression must reduce level"); ++checks;
            s={};s.deess=1;
            check(energy(render(s,rate,10000,0.8f),static_cast<size_t>(rate/2))<0.75f*energy(render({},rate,10000,0.8f)),"de-esser must attenuate highs"); ++checks;
            s={};s.tone=1;
            check(energy(render(s,rate,440,0.8f),static_cast<size_t>(rate/2))<0.9f*energy(dry),"saturation must change signal"); ++checks;
            s={};s.space=1;s.echo=1;
            const auto reference=render(s,rate,440,0.5f,1);
            for(int block : {32,128,511,2048}) {
                const auto actual=render(s,rate,440,0.5f,block);
                for(size_t i=0;i<actual.size();++i) { check(std::abs(actual[i]-reference[i])<1.e-6f,"block-size-dependent DSP"); }
                ++checks;
            }
        }
        for(bool reverb : {false,true}) {
            d2nz::VocalChain chain; chain.prepare(48000);
            d2nz::Settings s; if(reverb)s.space=1;else s.echo=1; chain.setSettings(s);
            std::vector<float> audio(96000,0);float* p=audio.data(); chain.process(&p,1,48000);
            audio[0]=1; chain.process(&p,1,96000);
            check(energy(audio,1)>0,"space/echo must create an audible tail"); ++checks;
            chain.reset(); std::fill(audio.begin(),audio.end(),0); chain.process(&p,1,96000);
            check(energy(audio)==0,"reset must clear tails"); ++checks;
        }
        d2nz::VocalChain chain;chain.prepare(48000);
        d2nz::Settings s;s.power=1;chain.setSettings(s);
        std::vector<float> left(48000,0.8f),right(48000,0.2f);float* ptrs[]={left.data(),right.data()};
        auto meters=chain.process(ptrs,2,48000);
        check(meters.reductionDb>10,"gain reduction meter must reflect compression");
        check(std::abs(left.back()/right.back()-4.f)<0.001f,"stereo compressor must remain linked");checks+=2;
        std::cout<<"PASS "<<checks<<" DSP checks: transparency, gain, cleanup, compression, de-essing, saturation, tails, reset, block invariance, stereo linking\n";
        return EXIT_SUCCESS;
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return EXIT_FAILURE;}
}
