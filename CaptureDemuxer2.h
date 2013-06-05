// CaptureDemuxer2.h

#ifndef _PPBOX_CAPTURE_CAPTURE_DEMUXER2_H_
#define _PPBOX_CAPTURE_CAPTURE_DEMUXER2_H_

#include "ppbox/demux/packet/PacketDemuxer.h"

namespace ppbox
{
    namespace capture
    {

        class CaptureFilter2;

        class CaptureDemuxer2
            : public ppbox::demux::PacketDemuxer
        {
        public:
            CaptureDemuxer2(
                boost::asio::io_service & io_svc, 
                ppbox::data::PacketMedia & media);

            virtual ~CaptureDemuxer2();

        public:
            virtual bool free_sample(
                ppbox::demux::Sample & sample, 
                boost::system::error_code & ec);

        protected:
            virtual bool check_open(
                boost::system::error_code & ec);

        private:
            CaptureFilter2 * filter_;
            std::vector<bool> stream_begs_;
        };

        PPBOX_REGISTER_PACKET_DEMUXER("capture2", CaptureDemuxer2);

    } // namespace capture
} // namespace ppbox

#endif // _PPBOX_CAPTURE_CAPTURE_DEMUXER2_H_
