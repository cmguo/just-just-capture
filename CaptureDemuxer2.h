// CaptureDemuxer2.h

#ifndef _JUST_CAPTURE_CAPTURE_DEMUXER2_H_
#define _JUST_CAPTURE_CAPTURE_DEMUXER2_H_

#include "just/demux/packet/PacketDemuxer.h"

namespace just
{
    namespace capture
    {

        class CaptureFilter2;

        class CaptureDemuxer2
            : public just::demux::PacketDemuxer
        {
        public:
            CaptureDemuxer2(
                boost::asio::io_service & io_svc, 
                just::data::PacketMedia & media);

            virtual ~CaptureDemuxer2();

        public:
            virtual bool free_sample(
                just::demux::Sample & sample, 
                boost::system::error_code & ec);

        protected:
            virtual bool check_open(
                boost::system::error_code & ec);

        private:
            CaptureFilter2 * filter_;
            std::vector<bool> stream_begs_;
        };

        JUST_REGISTER_PACKET_DEMUXER("capture2", CaptureDemuxer2);

    } // namespace capture
} // namespace just

#endif // _JUST_CAPTURE_CAPTURE_DEMUXER2_H_
