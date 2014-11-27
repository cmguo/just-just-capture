// CaptureDemuxer.h

#ifndef _JUST_CAPTURE_CAPTURE_DEMUXER_H_
#define _JUST_CAPTURE_CAPTURE_DEMUXER_H_

#include "just/demux/packet/PacketDemuxer.h"

namespace just
{
    namespace capture
    {

        class CaptureFilter;

        class CaptureDemuxer
            : public just::demux::PacketDemuxer
        {
        public:
            CaptureDemuxer(
                boost::asio::io_service & io_svc, 
                just::data::PacketMedia & media);

            virtual ~CaptureDemuxer();

        public:
            virtual bool free_sample(
                just::demux::Sample & sample, 
                boost::system::error_code & ec);

        protected:
            virtual bool check_open(
                boost::system::error_code & ec);

        private:
            CaptureFilter * filter_;
            std::vector<bool> stream_begs_;
        };

        JUST_REGISTER_PACKET_DEMUXER("capture", CaptureDemuxer);

    } // namespace capture
} // namespace just

#endif // _JUST_CAPTURE_CAPTURE_DEMUXER_H_
