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
        {
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
            if (cycle_.push(sample)) {
                if (!beg_) {
                    stream_eofs_[sample.itrack] = true;
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

        bool CaptureSource::get_streams(
            std::vector<CaptureStream> & streams, 
            boost::system::error_code & ec)
        {
            streams = streams_;
            ec.clear();
            return true;
        }

        bool CaptureSource::free_sample(
            void const * context, 
            boost::system::error_code & ec)
        {
            ec.clear();
            return config_.free_sample(context);
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

        bool CaptureSource::continuable(
            boost::system::error_code const & ec)
        {
            return ec == boost::asio::error::would_block;
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
            size_t size = sizeof(CaptureSample);
            if (sample.buffer == NULL) {
                if (buffers_.size() < sample.size) {
                    buffers_.resize(sample.size);
                }
                config_.get_sample_buffers(sample.context, &buffers_.front());
                CaptureBuffer buffers2[2];
                buffers2[0].data = (boost::uint8_t const *)&sample;
                buffers2[0].len = sizeof(sample);
                buffers2[1].data = (boost::uint8_t const *)&buffers_.front();
                buffers2[1].len = sizeof(CaptureBuffer) * sample.size;
                util::buffers::buffers_copy(buffers,  framework::container::make_array(buffers2));
                size += sizeof(CaptureBuffer) * sample.size;
            } else {
                util::buffers::buffers_copy(buffers,  boost::asio::buffer(&sample, sizeof(sample)));
            }
            cycle_.pop();
            return size;
        }
        
        void CaptureSource::response(
            boost::system::error_code const & ec)
        {
            if (!resp_.empty()) {
                response_type resp;
                resp.swap(resp_);
                resp(ec);
            }
        }

    } // namespace capture
} // namespace ppbox
