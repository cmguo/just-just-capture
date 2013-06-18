// CaptureFilter.h

#ifndef _PPBOX_CAPTURE_RTM_FILTER_H_
#define _PPBOX_CAPTURE_RTM_FILTER_H_

#include <ppbox/demux/base/DemuxBase.h>
#include <ppbox/data/packet/PacketFeature.h>

#include <boost/asio/buffer.hpp>

namespace ppbox
{
    namespace capture
    {

        using ppbox::data::PacketFeature;

        struct CaptureBuffer
        {
            CaptureBuffer()
                : data(NULL)
                , len(0)
            {
            }

            CaptureBuffer & operator=(
                boost::asio::const_buffer const & b)
            {
                data = boost::asio::buffer_cast<boost::uint8_t const *>(b);
                len = boost::asio::buffer_size(b);
                return *this;
            }

            boost::uint8_t const * data;
            boost::uint32_t len;
        };

        typedef ppbox::demux::StreamInfo CaptureStream;

        struct CaptureSample
        {
            CaptureSample()
                : itrack(boost::uint32_t(-1))
                , flags(0)
                , dts(0)
                , cts_delta(0)
                , duration(0)
                , size(0)
                , buffer(NULL)
                , context(NULL)
            {
            }

            boost::uint32_t itrack;
            boost::uint32_t flags;
            boost::uint64_t dts;
            boost::uint32_t cts_delta;
            boost::uint32_t duration;
            boost::uint32_t size;
            boost::uint8_t const * buffer;
            void const * context;

            boost::uint32_t real_size() const
            {
                return sizeof(CaptureSample) + sizeof(CaptureBuffer) * (size);
            }
        };

        struct CaptureConfigData
        {
            CaptureConfigData()
                : stream_count(0)
                , flags(0)
                , get_sample_buffers(NULL)
                , free_sample(NULL)
            {
            }

            enum FlagEnum
            {
                f_stream_ordered  = 1, 
                f_multi_thread    = 2, 
            };

            boost::uint32_t stream_count;
            boost::uint32_t flags;
            bool (*get_sample_buffers)(void const *, CaptureBuffer *);
            bool (*free_sample)(void const *);
        };

    } // namespace mux
} // namespace ppbox

namespace boost
{
    namespace asio
    {
        inline boost::asio::const_buffers_1 buffer(ppbox::capture::CaptureBuffer const & buf)
        {
            return buffer(buf.data, buf.len);
        }
    }
}

#endif // _PPBOX_CAPTURE_RTM_FILTER_H_
