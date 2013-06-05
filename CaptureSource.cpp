// CaptureSource.cpp

#include "ppbox/capture/Common.h"
#include "ppbox/capture/CaptureSource.h"
#include "ppbox/capture/CaptureFormat.h"

#include <ppbox/data/base/SourceError.h>

#include <util/buffers/BuffersCopy.h>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/logger/StringRecord.h>
#include <framework/container/Array.h>

#include <boost/bind.hpp>

namespace ppbox
{
    namespace capture
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.capture.CaptureSource", framework::logger::Debug);

        CaptureSource::CaptureSource(
            boost::asio::io_service & io_svc)
            : ppbox::data::UrlSource(io_svc)
            , beg_(false)
            , eof_(false)
            , cycle_(1024)
            , free_pieces_(1024 * 3)
            , free_pieces2_(NULL)
        {
            feature_.piece_size = 256;
            feature_.block_size = 256 * 1024;
        }

        CaptureSource::~CaptureSource()
        {
        }

        bool CaptureSource::init(
            CaptureConfigData const & config, 
            boost::system::error_code & ec)
        {
            if (!streams_.empty()) {
                ec = framework::system::logic_error::item_already_exist;
                return false;
            }
            config_ = config;
            if (config_.get_sample_buffers == NULL) {
                config_.get_sample_buffers = s_get_sample_buffers;
            }
            if (config_.free_sample == NULL) {
                config_.free_sample = s_free_sample;
                Piece * p = alloc_piece();
                free_pieces_.push(p);
                while ((p = free_pieces2_)) {
                    free_pieces2_ = free_pieces2_->next;
                    p->next = NULL;
                    free_pieces_.push(p);
                }
            }
            boost::uint32_t count = config_.stream_count; 
            streams_.resize(count);
            stream_begs_.resize(count, false);
            stream_eofs_.resize(count, false);
            ec.clear();
            return true;
        }

        bool CaptureSource::set_stream(
            CaptureStream const & stream, 
            boost::system::error_code & ec)
        {
            if (streams_.size() < stream.index + 1) {
                ec = framework::system::logic_error::out_of_range;
                return false;
            }
            streams_[stream.index] = stream;
            ec.clear();
            return true;
        }

        bool CaptureSource::put_sample(
            CaptureSample const & sample, 
            boost::system::error_code & ec)
        {
            if (streams_.size() < sample.itrack + 1) {
                ec = framework::system::logic_error::out_of_range;
                return false;
            }
            CaptureSample sample2 = sample; 
            if (sample2.context == NULL) {
                assert(sample2.buffer);
                sample2.context = copy_sample_buffers(sample2.size, sample2.buffer);
            }
            if (cycle_.push(sample2)) {
                if (!beg_) {
                    stream_eofs_[sample2.itrack] = true;
                    if (std::find(stream_eofs_.begin(), stream_eofs_.end(), false) == stream_eofs_.end()) {
                        beg_ = true;
                        response(boost::system::error_code());
                    }
                }
                ec.clear();
                return true;
            } else {
                ec = boost::asio::error::no_buffer_space;
                return false;
            }
        }

        bool CaptureSource::stream_eof(
            boost::uint32_t index, 
            boost::system::error_code & ec)
        {
            if (index >= stream_eofs_.size()) {
                ec = framework::system::logic_error::out_of_range;
                return false;
            }
            stream_eofs_[index] = true;
            if (std::find(stream_eofs_.begin(), stream_eofs_.end(), false) == stream_eofs_.end()) {
                eof_ = true;
            }
            ec.clear();
            return true;
        }

        bool CaptureSource::term()
        {
            response(boost::asio::error::operation_aborted);
            eof_ = true;
            return true;
        }

        bool CaptureSource::get_streams(
            std::vector<CaptureStream> & streams, 
            boost::system::error_code & ec)
        {
            streams = streams_;
            ec.clear();
            return true;
        }

        bool CaptureSource::get_sample_buffers(
            CaptureSample const & sample, 
            std::vector<CaptureBuffer> & buffers, 
            boost::system::error_code & ec)
        {
            ec.clear();
            return config_.get_sample_buffers(sample.context, &buffers.at(0));
        }

        bool CaptureSource::free_sample(
            CaptureSample const & sample, 
            boost::system::error_code & ec)
        {
            ec.clear();
            return config_.free_sample(sample.context);
        }

        boost::system::error_code CaptureSource::open(
            framework::string::Url const & url, 
            boost::uint64_t beg, 
            boost::uint64_t end, 
            boost::system::error_code & ec)
        {
            return ec;
        }

        void CaptureSource::async_open(
            framework::string::Url const & url, 
            boost::uint64_t beg, 
            boost::uint64_t end, 
            response_type const & resp)
        {
            resp_ = resp;
        }

        bool CaptureSource::is_open(
            boost::system::error_code & ec)
        {
            ec.clear();
            return true;
        }

        boost::system::error_code CaptureSource::close(
            boost::system::error_code & ec)
        {
            cycle_.clear();
            ec.clear();
            return ec;
        }

        boost::system::error_code CaptureSource::cancel(
            boost::system::error_code & ec)
        {
            response(boost::asio::error::operation_aborted);
            ec.clear();
            return ec;
        }

        boost::system::error_code CaptureSource::set_non_block(
            bool non_block, 
            boost::system::error_code & ec)
        {
            ec.clear();
            return ec;
        }

        boost::system::error_code CaptureSource::set_time_out(
            boost::uint32_t time_out, 
            boost::system::error_code & ec)
        {
            ec.clear();
            return ec;
        }

        size_t CaptureSource::private_read_some(
            buffers_t const & buffers, 
            boost::system::error_code & ec)
        {
            if (cycle_.empty()) {
                if (eof_) {
                    ec = boost::asio::error::eof;
                } else {
                    ec = boost::asio::error::would_block;
                }
                return 0;
            }
            CaptureSample & sample = cycle_.front();
            util::buffers::buffers_copy(buffers,  boost::asio::buffer(&sample, sizeof(sample)));
            cycle_.pop();
            return sizeof(CaptureSample);
        }
        
        void CaptureSource::response(
            boost::system::error_code const & ec)
        {
            if (!resp_.empty()) {
                response_type resp;
                resp.swap(resp_);
                get_io_service().post(boost::bind(resp, ec));
            }
        }

        CaptureSource::Piece * CaptureSource::alloc_pieces(
            size_t count)
        {
            Piece * p = NULL;
            if (free_pieces_.size() < count) {
                Piece ** pp = &p;
                for (size_t i = 0; i < count; ++i) {
                    *pp = alloc_piece();
                    pp = &(*pp)->next;
                }
                assert(*pp == NULL);
            } else {
                framework::container::SafeCycle<Piece *>::const_iterator iter = free_pieces_.begin();
                Piece ** pp = &p;
                for (size_t i = 0; i < count; ++i, ++iter) {
                    *pp = *iter;
                    pp = &(*pp)->next;
                }
                free_pieces_.pop(count);
                assert(*pp == NULL);
            }
            return p;
        }

        void CaptureSource::free_pieces(
            Piece * list)
        {
            while (list) {
                Piece * p = list;
                list = list->next;
                p->next = NULL;
                if (!free_pieces_.push(p))
                    free_piece(p);
            }
        }

        CaptureSource::Piece * CaptureSource::alloc_piece()
        {
            boost::mutex::scoped_lock lc(mutex_);
            if (free_pieces2_ == NULL) {
                void * ptr = memory_.alloc_block(feature_.block_size);
                if (ptr == NULL) {
                    assert(false);
                    return NULL;
                }
                blocks_.push_back(ptr);
                LOG_DEBUG("[alloc_piece] block count = " << blocks_.size());
                void * end = (char *)ptr + feature_.block_size - feature_.piece_size - sizeof(Piece);
                while (ptr <= end) {
                    Piece * p = (Piece *)ptr;
                    ptr = ((char *)(ptr) + sizeof(Piece) + feature_.piece_size);
                    p->next = free_pieces2_;
                    free_pieces2_ = p;
                }
            }
            Piece * p = free_pieces2_;
            free_pieces2_ = free_pieces2_->next;
            p->next = NULL;
            return p;
        }

        void CaptureSource::free_piece(
            Piece * piece)
        {
            boost::mutex::scoped_lock lc(mutex_);
            piece->next = free_pieces2_;
            free_pieces2_ = piece;
        }

        bool CaptureSource::s_get_sample_buffers(
            void const * context, 
            CaptureBuffer * buffers)
        {
            Piece const * p = (Piece const *)context;
            Packet const * pkt = (Packet const *)(p + 1);
            return pkt->owner->get_sample_buffers(p, buffers);
        }

        bool CaptureSource::s_free_sample(
            void const * context)
        {
            Piece const * p = (Piece const *)context;
            Packet const * pkt = (Packet const *)(p + 1);
            return pkt->owner->free_sample(p);
        }

        struct CaptureSource::piece_list
        {
            typedef piece_list const_iterator;
            piece_list(Piece * p = NULL, size_t s = 0): p_(p), s_(s) {}
            const_iterator begin() const { return *this; }
            const_iterator end() const { return const_iterator(); }
            boost::asio::mutable_buffer operator*() { return boost::asio::mutable_buffer(p_ + 1, s_); }
            const_iterator operator++() { const_iterator i = *this; p_ = p_->next; return i; }
            const_iterator & operator++(int) { p_ = p_->next; return *this; }
            friend bool operator==(const_iterator const & l, const_iterator const & r) { return l.p_ == r.p_; }
            friend bool operator!=(const_iterator const & l, const_iterator const & r) { return l.p_ != r.p_; }
        private:
            Piece * p_;
            size_t s_;
        };

        CaptureSource::Piece const * CaptureSource::copy_sample_buffers(
            boost::uint32_t & size, 
            boost::uint8_t const * & buffer)
        {
            boost::uint32_t count = (sizeof(Packet) + size + feature_.piece_size - 1) / feature_.piece_size;
            Piece * p = alloc_pieces(count);
            Packet pkt = {size, this};
            boost::asio::const_buffer buffers[2] = {
                boost::asio::buffer(&pkt, sizeof(pkt)), 
                boost::asio::buffer(buffer, size), 
            };
            util::buffers::buffers_copy(
                piece_list(p, feature_.piece_size), 
                framework::container::make_array(buffers));
            if (count == 1) {
                buffer = (boost::uint8_t const *)p + sizeof(Piece) + sizeof(Packet);
            } else {
                size = count;
                buffer = NULL;
            }
            return p;
        }

        bool CaptureSource::get_sample_buffers(
            Piece const * p, 
            CaptureBuffer * buffers)
        {
            Packet * pkt = (Packet *)(p + 1);
            boost::uint32_t left = sizeof(Packet) + pkt->size;
            CaptureBuffer * pb = buffers;
            while (left > feature_.piece_size) {
                pb->data = (boost::uint8_t const *)(p + 1);
                pb->len = feature_.piece_size;
                p = p->next;
                ++pb;
                left -= feature_.piece_size;
            }
            pb->data = (boost::uint8_t const *)(p + 1);
            pb->len = left;
            buffers->data += sizeof(Packet);
            buffers->len -= sizeof(Packet);
            assert(p->next == NULL);
            return true;
        }

        bool CaptureSource::free_sample(
            Piece const * p)
        {
            free_pieces(const_cast<Piece *>(p));
            return true;
        }

    } // namespace capture
} // namespace ppbox
