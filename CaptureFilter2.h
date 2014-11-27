// CaptureFilter2.h

#ifndef _JUST_CAPTURE_CAPTURE_FILTER2_H_
#define _JUST_CAPTURE_CAPTURE_FILTER2_H_

#include "just/capture/CaptureFormat.h"

#include <just/demux/packet/Filter.h>

namespace just
{
    namespace capture
    {

        class CaptureSource2;

        class CaptureFilter2
            : public just::demux::Filter
        {
        public:
            CaptureFilter2(
                CaptureSource2 & source);

            ~CaptureFilter2();

        public:
            virtual bool get_sample(
                just::demux::Sample & sample,
                boost::system::error_code & ec);

            virtual bool get_next_sample(
                just::demux::Sample & sample,
                boost::system::error_code & ec);

            virtual bool get_last_sample(
                just::demux::Sample & sample,
                boost::system::error_code & ec);

        private:
            void parse_for_time(
                just::demux::Sample & sample,
                boost::system::error_code & ec);

        private:
            CaptureSource2 & source_;
            std::vector<CaptureBuffer> buffers_;
        };

    } // namespace capture
} // namespace just

#endif // _JUST_CAPTURE_CAPTURE_FILTER2_H_
