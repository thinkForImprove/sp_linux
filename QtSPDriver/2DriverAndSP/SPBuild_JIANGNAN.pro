TEMPLATE = subdirs

# 定制编译顺序
CONFIG += ordered

# 子项目，一起编译时，是按此顺序编译
SUBDIRS += \
    SPBuild/AllDevPort \
    SPBuild/BCRBuild.pro \
    SPBuild_JIANGNAN/CAMBuild.pro \
    SPBuild/CPRBuild.pro \
    SPBuild/CSRBuild.pro \
    SPBuild/DSRBuild.pro \
    SPBuild_SXXH/FIDCBuild.pro \
    SPBuild/FIGBuild.pro \
    SPBuild_SXXH/IDCBuild.pro \
    SPBuild/IDXBuild.pro \
    SPBuild/PINBuild.pro \
    SPBuild/PTRBuild.pro \
    SPBuild/SIGBuild.pro \
    SPBuild_JIANGNAN/SIUBuild.pro \
    SPBuild/UKEYBuild.pro
