// CaptureFilter2.h

#ifndef _PPBOX_CAPTURE_CAPTURE_FILTER2_H_
#define _PPBOX_CAPTURE_CAPTURE_FILTER2_H_

#include "ppbox/capture/CaptureFormat.h"

#include <ppbox/demux/packet/Filter.h>

namespace ppbox
{
    namespace capture
    {

        class CaptureSource2;

        class CaptureFilter2
            : public ppbox::demux::Filter
        {
        public:
            CaptureFilter2(
                CaptureSource2 & source);

            ~CaptureFilter2();

        public:
            virtual bool get_sample(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec);

            virtual bool get_next_sample(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec);

            virtual bool get_last_sample(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec);

        private:
            void parse_for_time(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec);

        private:
            CaptureSource2 & source_;
            std::vector<CaptureBuffer> buffers_;
        };

    } // namespace capture
} // namespace ppbox

#endif // _PPBOX_CAPTURE_CAPTURE_FILTER2_H_
