// CaptureSource.h

#ifndef _PPBOX_CAPTURE_CAPTURE_SOURCE_H_
#define _PPBOX_CAPTURE_CAPTURE_SOURCE_H_

#include "ppbox/capture/CaptureFormat.h"

#include <ppbox/data/base/UrlSource.h>
#include <ppbox/data/packet/PacketFeature.h>

#include <framework/container/SafeCycle.h>
#include <framework/memory/PrivateMemory.h>

#include <boost/thread/mutex.hpp>

namespace ppbox
{
    namespace capture
    {

        class CaptureSource
            : public ppbox::data::UrlSource
        {
        public:
            CaptureSource(
                boost::asio::io_service & io_svc);

            virtual ~CaptureSource();

        public:
            bool init(
                CaptureConfigData const & config, 
                boost::system::error_code & ec);

            bool set_stream(
                CaptureStream const & stream, 
                boost::system::error_code & ec);

            bool put_sample(
                CaptureSample const & sample, 
                boost::system::error_code & ec);

            bool stream_eof(
                boost::uint32_t index, 
                boost::system::error_code & ec);

            bool term();

        public:
            bool get_streams(
                std::vector<CaptureStream> & streams, 
                boost::system::error_code & ec);

            bool get_sample_buffers(
                CaptureSample const & sample, 
                std::vector<CaptureBuffer> & buffers, 
                boost::system::error_code & ec);

            bool free_sample(
                CaptureSample const & sample, 
                boost::system::error_code & ec);

            CaptureConfigData const & config() const
            {
                return config_;
            }

            PacketFeature const & feature() const
            {
                return feature_;
            }

        public:
            using ppbox::data::UrlSource::open;

            virtual boost::system::error_code open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                boost::system::error_code & ec);

            using ppbox::data::UrlSource::async_open;

            virtual void async_open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                response_type const & resp);

            virtual bool is_open(
                boost::system::error_code & ec);

            virtual boost::system::error_code close(
                boost::system::error_code & ec);

        public:
            virtual boost::system::error_code cancel(
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
            void response(
                boost::system::error_code const & ec);

        private:
            struct StreamSamples;

            struct Packet
            {
                boost::uint32_t size;
                CaptureSource * owner;
                StreamSamples * stream;
            };

            struct Piece
            {
                Piece * next;
            };

            struct piece_list;

        private:
            Piece * alloc_pieces(
                StreamSamples & stream_samples, 
                size_t count);

            void free_pieces(
                StreamSamples & stream_samples, 
                Piece * list);

            Piece * alloc_piece();

            void free_piece(
                Piece * list);

        private:
            static bool s_get_sample_buffers(
                void const * context, 
                CaptureBuffer * buffers);

            static bool s_free_sample(
                void const * context);

            Piece const * copy_sample_buffers(
                StreamSamples & stream_samples, 
                boost::uint32_t & size, 
                boost::uint8_t const * & buffer);

            bool get_sample_buffers(
                Piece const * context, 
                CaptureBuffer * buffers);

            bool free_sample(
                StreamSamples & stream_samples, 
                Piece const * context);

        private:
            CaptureConfigData config_; 
            response_type resp_;
            PacketFeature feature_;
            std::vector<CaptureStream> streams_;
            std::vector<bool> stream_begs_;
            std::vector<bool> stream_eofs_;
            bool beg_;
            bool eof_;
            framework::memory::PrivateMemory memory_;
            boost::mutex mutex_;
            Piece * free_pieces2_;
            std::vector<void *> blocks_;
            StreamSamples * stream_samples_;
        };

        PPBOX_REGISTER_URL_SOURCE("capture", CaptureSource);

    } // namespace capture
} // namespace ppbox

#endif // _PPBOX_CAPTURE_CAPTURE_SOURCE_H_
