TEMPLATE = subdirs

# 定制编译顺序
CONFIG += ordered

# 子项目，一起编译时，是按此顺序编译
SUBDIRS += \
    SPBuild/AllDevPort \
    SPBuild/BCRBuild.pro \
    SPBuild_SXXH/BRMBuild.pro \
    SPBuild_SXXH/CAMBuild.pro \
    SPBuild/CORBuild.pro \
    SPBuild/CPRBuild.pro \
    SPBuild/CSRBuild.pro \
    SPBuild/DEPBuild.pro \
    SPBuild/DPRBuild.pro \
    SPBuild_SXXH/FIDCBuild.pro \
    SPBuild/FIGBuild.pro \
    SPBuild_SXXH/IDCBuild.pro \
    SPBuild/IDXBuild.pro \
    SPBuild/MSRBuild.pro \
    SPBuild_SXXH/PINBuild.pro \
    SPBuild/PTRBuild.pro \
    SPBuild/PPRBuild.pro \
    SPBuild/SIGBuild.pro \
    SPBuild_SXXH/SIUBuild.pro \
    SPBuild/UKEYBuild.pro
