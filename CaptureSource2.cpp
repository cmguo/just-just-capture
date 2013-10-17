// CaptureSource2.cpp

#include "ppbox/capture/Common.h"
#include "ppbox/capture/CaptureSource2.h"
#include "ppbox/capture/CaptureFormat.h"

#include <ppbox/avcodec/Capture.h>
using namespace ppbox::avcodec;

#include <util/buffers/BuffersCopy.h>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/string/ParseStl.h>
#include <framework/container/Array.h>

namespace ppbox
{
    namespace capture
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.capture.CaptureSource2", framework::logger::Debug);

        struct CaptureSource2::StreamStatus
        {
            StreamStatus()
                : capture(NULL)
                , time(0)
                , time_mul(0)
            {
            }

            Capture * capture;
            boost::uint64_t time;
            boost::uint64_t time_mul;
            Sample sample;
        };

        CaptureSource2::CaptureSource2(
            boost::asio::io_service & io_svc)
            : util::stream::UrlSource(io_svc)
        {
            feature_.piece_size = 256;
            feature_.packet_max_size = 256;
            feature_.block_size = 256 * 1024;
        }

        CaptureSource2::~CaptureSource2()
        {
        }

        bool CaptureSource2::get_streams(
            std::vector<CaptureStream> & streams, 
            boost::system::error_code & ec)
        {
            streams = streams_;
            ec.clear();
            return true;
        }

        bool CaptureSource2::free_sample(
            CaptureSample const & sample, 
            boost::system::error_code & ec)
        {
            if (sample.context) {
                Sample sample2;
                sample2.context = sample.context;
                return status_[sample.itrack].capture->free(sample2, ec);
            }
            return true;
        }

        static boost::uint64_t common_divisor(
            boost::uint64_t scale_in, 
            boost::uint64_t scale_out)
        {
            while (scale_in != scale_out) {
                if (scale_in > scale_out) {
                    scale_in -= scale_in / scale_out * scale_out;
                    if (scale_in == 0)
                        return scale_out;
                } else {
                    scale_out -= scale_out / scale_in * scale_in;
                    if (scale_out == 0)
                        return scale_in;
                }
            }
            return scale_in;
        }

        bool CaptureSource2::open(
            framework::string::Url const & url, 
            boost::uint64_t beg, 
            boost::uint64_t end, 
            boost::system::error_code & ec)
        {
            std::string video = url.param("video");
            if (!video.empty()) {
                init_stream(video, ec);
            }
            std::string audio = url.param("audio");
            if (!audio.empty()) {
                init_stream(audio, ec);
            }
            if (streams_.empty()) {
                return false;
            }
            boost::uint64_t divisor = streams_[0].time_scale;
            boost::uint64_t multiple = streams_[0].time_scale;
            for (size_t i = 1 ; i < streams_.size(); ++i) {
                divisor = common_divisor(divisor, streams_[i].time_scale);
                multiple *= (streams_[i].time_scale / divisor);
            }
            for (size_t i = 0 ; i < status_.size(); ++i) {
                status_[i].time_mul = multiple / streams_[i].time_scale;
                status_sort_.push_back(&status_[i]);
            }
            feature_.packet_max_size += sizeof(Sample);
            feature_.piece_size = feature_.packet_max_size;
            feature_.block_size = 4 * 1024 * 1024;
            feature_.buffer_size = 8 * 1024 * 1024;
            return true;
        }

        bool CaptureSource2::is_open(
            boost::system::error_code & ec)
        {
            ec.clear();
            return true;
        }

        bool CaptureSource2::close(
            boost::system::error_code & ec)
        {
            ec.clear();
            return true;
        }

        bool CaptureSource2::set_non_block(
            bool non_block, 
            boost::system::error_code & ec)
        {
            ec.clear();
            return true;
        }

        bool CaptureSource2::set_time_out(
            boost::uint32_t time_out, 
            boost::system::error_code & ec)
        {
            ec.clear();
            return true;
        }

        size_t CaptureSource2::private_read_some(
            buffers_t const & buffers, 
            boost::system::error_code & ec)
        {
            if (status_sort_.empty()) {
                ec = boost::asio::error::eof;
                return 0;
            }
            StreamStatus * stat = status_sort_[0];
            stat->sample.data.clear();
            if (!stat->capture->get(stat->sample, ec)) {
                return 0;
            }
            CaptureSample sample;
            sample.itrack = stat->sample.itrack;
            sample.flags = stat->sample.flags;
            sample.dts = stat->sample.dts;
            sample.cts_delta = stat->sample.cts_delta;
            sample.duration = stat->sample.duration;
            sample.size = stat->sample.size;
            sample.context = stat->sample.context;
            if (sample.context == NULL) {
                if (buffers_.size() < stat->sample.data.size() + 1)
                    buffers_.resize(stat->sample.data.size() + 1);
                buffers_[0].data = (boost::uint8_t const *)&sample;
                buffers_[0].len = sizeof(sample);
                for (size_t i = 0; i < stat->sample.data.size(); ++i) {
                    buffers_[i + 1] = stat->sample.data[i];
                }
                sample.size = util::buffers::buffers_copy(buffers, buffers_, sizeof(sample) + sample.size);
            } else {
                if (buffers_.size() < stat->sample.data.size())
                    buffers_.resize(stat->sample.data.size());
                for (size_t i = 0; i < stat->sample.data.size(); ++i) {
                    buffers_[i] = stat->sample.data[i];
                }
                CaptureBuffer buffers2[2];
                buffers2[0].data = (boost::uint8_t const *)&sample;
                buffers2[0].len = sizeof(sample);
                buffers2[1].data = (boost::uint8_t const *)&buffers_;
                buffers2[1].len = sizeof(CaptureBuffer) * stat->sample.data.size();
                sample.size = util::buffers::buffers_copy(buffers, framework::container::make_array(buffers2));
            }
            stat->sample.dts += stat->sample.duration;
            stat->time = stat->sample.dts * stat->time_mul;
            for (size_t i = 1; i <= status_sort_.size(); ++i) {
                if (i == status_sort_.size() || stat->time < status_sort_[i]->time) {
                    status_sort_[i - 1] = stat;
                    break;
                } else {
                    status_sort_[i - 1] = status_sort_[i];
                }
            }
            return sample.size;
        }

        bool CaptureSource2::init_stream(
            std::string const & device, 
            boost::system::error_code & ec)
        {
            std::string::size_type pos = device.find('{');
            std::string name = device.substr(0, pos);
            Capture * capture = ppbox::avcodec::CaptureFactory::create(name, ec);
            if (capture) {
                std::map<std::string, std::string> param_map;
                boost::system::error_code ec = 
                    framework::string::parse2(device.substr(pos), param_map);
                capture->open(param_map, ec);
                StreamStatus stat;
                stat.capture = capture;
                StreamInfo info;
                capture->get(info, ec);
                CaptureConfig config;
                memcpy(&config, &info.format_data.at(0), sizeof(config));
                if (info.time_scale == 0) {
                    info.time_scale = config.frame_rate_num;
                    stat.sample.duration = config.frame_rate_den;
                }
                info.index = streams_.size();
                stat.sample.itrack = status_.size();
                streams_.push_back(info);
                status_.push_back(stat);
                if (config.max_frame_size > feature_.packet_max_size) {
                    feature_.packet_max_size = config.max_frame_size;
                }
            }
            return true;
        }
        
    } // namespace capture
} // namespace ppbox
