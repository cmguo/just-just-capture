// CaptureMedia2.h

#ifndef _PPBOX_CAPTURE_CAPTURE_MEDIA2_H_
#define _PPBOX_CAPTURE_CAPTURE_MEDIA2_H_

#include "ppbox/capture/CaptureSource2.h"

#include <ppbox/data/packet/PacketMedia.h>

namespace ppbox
{
    namespace capture
    {

        class CaptureMedia2
            : public ppbox::data::PacketMedia
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
                ppbox::avbase::MediaBasicInfo & info,
                boost::system::error_code & ec) const;

            virtual bool get_info(
                ppbox::avbase::MediaInfo & info,
                boost::system::error_code & ec) const;

        public:
            virtual bool get_packet_feature(
                ppbox::data::PacketFeature & feature,
                boost::system::error_code & ec) const;

            virtual util::stream::Source & source();

        private:
            void handle_open(
                boost::system::error_code const & ec, 
                MediaBase::response_type const & resp);

        private:
            CaptureSource2 * source_;
            ppbox::avbase::MediaInfo info_;
        };

        PPBOX_REGISTER_MEDIA_BY_PROTOCOL("capture2", CaptureMedia2);

    } // capture
} // ppbox

#endif // _PPBOX_CAPTURE_CAPTURE_MEDIA2_H_
