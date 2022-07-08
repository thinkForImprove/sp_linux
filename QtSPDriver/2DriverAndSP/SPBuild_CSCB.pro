TEMPLATE = subdirs

# 定制编译顺序
CONFIG += ordered

# 子项目，一起编译时，是按此顺序编译
SUBDIRS += \
    SPBuild/AllDevPort \
    SPBuild/BCRBuild.pro \
    SPBuild_CSCB/BRMBuild.pro \
    SPBuild/CAMBuild.pro \
    SPBuild/CORBuild.pro \
    SPBuild/CPRBuild.pro \
    SPBuild/CSRBuild.pro \
    SPBuild/DEPBuild.pro \
    SPBuild_CSCB/FIDCBuild.pro \
    SPBuild_CSCB/FIGBuild.pro \
    SPBuild_CSCB/IDCBuild.pro \
    SPBuild/IDXBuild.pro \
    SPBuild/MSRBuild.pro \
    SPBuild_CSCB/PINBuild.pro \
    SPBuild/PTRBuild.pro \
    SPBuild/PPRBuild.pro \
    SPBuild/SIGBuild.pro \
    SPBuild_CSCB/SIUBuild.pro
