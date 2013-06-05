// CaptureSource2.h

#ifndef _PPBOX_CAPTURE_CAPTURE_SOURCE2_H_
#define _PPBOX_CAPTURE_CAPTURE_SOURCE2_H_

#include "ppbox/capture/CaptureFormat.h"

#include <ppbox/data/base/UrlSource.h>
#include <ppbox/data/packet/PacketFeature.h>

namespace ppbox
{
    namespace capture
    {

        class CaptureSource2
            : public ppbox::data::UrlSource
        {
        public:
            CaptureSource2(
                boost::asio::io_service & io_svc);

            virtual ~CaptureSource2();

        public:
            bool get_streams(
                std::vector<CaptureStream> & streams, 
                boost::system::error_code & ec);

            bool free_sample(
                CaptureSample const & sample, 
                boost::system::error_code & ec);

            bool get_feature(
                ppbox::data::PacketFeature & feature, 
                boost::system::error_code & ec);

        public:
            using ppbox::data::UrlSource::open;

            virtual boost::system::error_code open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                boost::system::error_code & ec);

            virtual bool is_open(
                boost::system::error_code & ec);

            virtual boost::system::error_code close(
                boost::system::error_code & ec);

        public:
            virtual boost::system::error_code set_non_block(
                bool non_block, 
                boost::system::error_code & ec);

            virtual boost::system::error_code set_time_out(
                boost::uint32_t time_out, 
                boost::system::error_code & ec);

        private:
            virtual std::size_t private_read_some(
                buffers_t const & buffers, 
                boost::system::error_code & ec);

        private:
            bool init_stream(
                std::string const & device, 
                boost::system::error_code & ec);

        private:
            void response(
                boost::system::error_code const & ec);

        private:
            struct StreamStatus;

            ppbox::data::PacketFeature feature_;
            std::vector<CaptureStream> streams_;
            std::vector<StreamStatus> status_;
            std::vector<StreamStatus *> status_sort_;
            std::vector<CaptureBuffer> buffers_;
        };

        PPBOX_REGISTER_URL_SOURCE("capture2", CaptureSource2);

    } // namespace capture
} // namespace ppbox

#endif // _PPBOX_CAPTURE_CAPTURE_SOURCE2_H_
