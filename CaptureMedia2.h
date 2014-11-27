// CaptureMedia2.h

#ifndef _JUST_CAPTURE_CAPTURE_MEDIA2_H_
#define _JUST_CAPTURE_CAPTURE_MEDIA2_H_

#include "just/capture/CaptureSource2.h"

#include <just/data/packet/PacketMedia.h>

namespace just
{
    namespace capture
    {

        class CaptureMedia2
            : public just::data::PacketMedia
        {
        public:
            CaptureMedia2(
                boost::asio::io_service & io_svc,
                framework::string::Url const & url);

            virtual ~CaptureMedia2();

        public:
            virtual void async_open(
                response_type const & resp);

            virtual void cancel(
                boost::system::error_code & ec);

            virtual void close(
                boost::system::error_code & ec);

        public:
            virtual bool get_basic_info(
                just::avbase::MediaBasicInfo & info,
                boost::system::error_code & ec) const;

            virtual bool get_info(
                just::avbase::MediaInfo & info,
                boost::system::error_code & ec) const;

        public:
            virtual bool get_packet_feature(
                just::data::PacketFeature & feature,
                boost::system::error_code & ec) const;

            virtual util::stream::Source & source();

        private:
            void handle_open(
                boost::system::error_code const & ec, 
                MediaBase::response_type const & resp);

        private:
            CaptureSource2 * source_;
            just::avbase::MediaInfo info_;
        };

        JUST_REGISTER_MEDIA_BY_PROTOCOL("capture2", CaptureMedia2);

    } // capture
} // just

#endif // _JUST_CAPTURE_CAPTURE_MEDIA2_H_
