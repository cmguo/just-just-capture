// CaptureModule.cpp

#include "ppbox/capture/Common.h"
#include "ppbox/capture/CaptureModule.h"
#include "ppbox/capture/ClassRegister.h"

namespace ppbox
{
    namespace capture
    {

        CaptureModule::CaptureModule(
            util::daemon::Daemon & daemon)
            : ppbox::common::CommonModuleBase<CaptureModule>(daemon, "CaptureModule")
        {
        }

        CaptureModule::~CaptureModule()
        {
        }

        boost::system::error_code CaptureModule::startup()
        {
            boost::system::error_code ec;
            return ec;
        }

        void CaptureModule::shutdown()
        {
        }

        struct CaptureModule::CaptureInfo
        {
            enum StatusEnum
            {
                closed, 
                opening, 
                canceled, 
                opened, 
            };

            size_t nref;
            framework::string::Url play_link;
            CaptureSource * capture;

            CaptureInfo()
                : nref(0)
                , capture(NULL)
            {
            }

            struct find_by_play_link
            {
                find_by_play_link(
                    framework::string::Url const & play_link)
                    : play_link_(play_link)
                {
                }

                bool operator()(
                    CaptureInfo const * info)
                {
                    return info->play_link == play_link_;
                }

            private:
                framework::string::Url const & play_link_;
            };

            struct find_by_capture
            {
                find_by_capture(
                    CaptureSource * capture)
                    : capture_(capture)
                {
                }

                bool operator()(
                    CaptureInfo const * info)
                {
                    return info->capture == capture_;
                }

            private:
                CaptureSource * capture_;
            };
        };

        CaptureSource * CaptureModule::create(
            framework::string::Url const & play_link, 
            boost::system::error_code & ec)
        {
            boost::mutex::scoped_lock lc(mutex_);

            CaptureInfo * info = NULL;
            std::vector<CaptureInfo *>::iterator iter = 
                std::find_if(captures_.begin(), captures_.end(), CaptureInfo::find_by_play_link(play_link));
            if (iter == captures_.end()) {
                info = new CaptureInfo;
                info->play_link = play_link;
                info->capture = new CaptureSource(io_svc());
                captures_.push_back(info);
            } else {
                info = *iter;
            }
            ++info->nref;
            return info->capture;
        }

        bool CaptureModule::destroy(
            CaptureSource * capture, 
            boost::system::error_code & ec)
        {
            boost::mutex::scoped_lock lc(mutex_);

            std::vector<CaptureInfo *>::iterator iter = 
                std::find_if(captures_.begin(), captures_.end(), CaptureInfo::find_by_capture(capture));
            if (iter == captures_.end()) {
                ec == framework::system::logic_error::item_not_exist;
                return false;
            }

            CaptureInfo * info = *iter;

            info->play_link.path(""); // can't be found by playlink

            if (--info->nref == 0) {
                captures_.erase(iter);
                delete info->capture;
                delete info;
            }
            
            return true;
        }

    } // namespace capture
} // namespace ppbox
