// CaptureDemuxer.h

#ifndef _PPBOX_CAPTURE_CAPTURE_DEMUXER_H_
#define _PPBOX_CAPTURE_CAPTURE_DEMUXER_H_

#include "ppbox/demux/packet/PacketDemuxer.h"

namespace ppbox
{
    namespace capture
    {

        class CaptureFilter;

        class CaptureDemuxer
            : public ppbox::demux::PacketDemuxer
        {
        public:
            CaptureDemuxer(
                boost::asio::io_service & io_svc, 
                ppbox::data::PacketMedia & media);

            virtual ~CaptureDemuxer();

        public:
            virtual bool free_sample(
                ppbox::demux::Sample & sample, 
                boost::system::error_code & ec);

        protected:
            virtual bool check_open(
                boost::system::error_code & ec);

        private:
            CaptureFilter * filter_;
            std::vector<bool> stream_begs_;
        };

        PPBOX_REGISTER_PACKET_DEMUXER("capture", CaptureDemuxer);

    } // namespace capture
} // namespace ppbox

#endif // _PPBOX_CAPTURE_CAPTURE_DEMUXER_H_
