// CaptureFilter.h

#ifndef _JUST_CAPTURE_CAPTURE_FILTER_H_
#define _JUST_CAPTURE_CAPTURE_FILTER_H_

#include "just/capture/CaptureFormat.h"

#include <just/demux/packet/Filter.h>

namespace just
{
    namespace capture
    {

        class CaptureSource;

        class CaptureFilter
            : public just::demux::Filter
        {
        public:
            CaptureFilter(
                CaptureSource & source);

            ~CaptureFilter();

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
            CaptureSource & source_;
            std::vector<CaptureBuffer> buffers_;
        };

    } // namespace capture
} // namespace just

#endif // _JUST_CAPTURE_CAPTURE_FILTER_H_
