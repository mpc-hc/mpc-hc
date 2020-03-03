#pragma once

#include "DspChunk.h"

namespace SaneAudioRenderer
{
    class DspBase
    {
    public:

        virtual ~DspBase() = default;

        virtual std::wstring Name() = 0;

        virtual bool Active() = 0;

        virtual void Process(DspChunk& chunk) = 0;
        virtual void Finish(DspChunk& chunk) = 0;
    };
}
