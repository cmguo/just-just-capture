// CaptureFilter2.cpp

#include "ppbox/capture/Common.h"
#include "ppbox/capture/CaptureFilter2.h"
#include "ppbox/capture/CaptureSource2.h"

#include <ppbox/demux/base/DemuxError.h>
using namespace ppbox::demux;

#include <ppbox/data/base/MemoryLock.h>

#include <util/buffers/BuffersCopy.h>

namespace ppbox
{
    namespace capture
    {

        CaptureFilter2::CaptureFilter2(
            CaptureSource2 & source)
            : source_(source)
        {
        }

        CaptureFilter2::~CaptureFilter2()
        {
        }

        bool CaptureFilter2::get_sample(
            Sample & sample,
            boost::system::error_code & ec)
        {
            if (!Filter::get_sample(sample, ec))
                return false;

            CaptureSample const & header = 
                *(boost::asio::buffer_cast<CaptureSample const *>(sample.data.front()));

            assert(boost::asio::buffer_size(sample.data.front()) > sizeof(header));
            sample.data.front() = sample.data.front() + sizeof(header);

            sample.itrack = header.itrack;
            sample.flags = header.flags;
            sample.dts = header.dts;
            sample.cts_delta = header.cts_delta;
            sample.duration = header.duration;
            sample.memory->offset = (intptr_t)&header;

            if (header.context == NULL) {
                sample.size = header.size;
            } else {
                if (buffers_.size() < header.size) {
                    buffers_.resize(header.size);
                }
                util::buffers::buffers_copy(
                    boost::asio::buffer((boost::uint8_t *)&buffers_.at(0), sizeof(CaptureBuffer) * header.size), sample.data);
                sample.data.clear();
                boost::uint32_t size = 0;
                for (boost::uint32_t i = 0; i < header.size; ++i) {
                    sample.data.push_back(boost::asio::buffer(buffers_[i]));
                    size += buffers_[i].len;
                }
                sample.size = size;
            }

            return true;
        }

        bool CaptureFilter2::get_next_sample(
            Sample & sample,
            boost::system::error_code & ec)
        {
            if (!Filter::get_next_sample(sample, ec))
                return false;

            parse_for_time(sample, ec);

            return true;
        }

        bool CaptureFilter2::get_last_sample(
            Sample & sample,
            boost::system::error_code & ec)
        {
            if (!Filter::get_last_sample(sample, ec))
                return false;

            parse_for_time(sample, ec);

            return true;
        }

        void CaptureFilter2::parse_for_time(
            Sample & sample,
            boost::system::error_code & ec)
        {
            CaptureSample const & header = 
                *(boost::asio::buffer_cast<CaptureSample const *>(sample.data.front()));

            sample.dts = header.dts;
        }

    } // namespace mux
} // namespace ppbox
