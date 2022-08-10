TEMPLATE = subdirs

# 定制编译顺序
CONFIG += ordered

# 子项目，一起编译时，是按此顺序编译
SUBDIRS += \
    SPBuild_OPTIM/BCRBuild.pro \
    SPBuild_OPTIM/CAMBuild.pro \
    SPBuild_OPTIM/FIDCBuild.pro \
    SPBuild_OPTIM/IDCBuild.pro \
    SPBuild_OPTIM/IDXBuild.pro \
    SPBuild_OPTIM/MSRBuild.pro \
    SPBuild_OPTIM/SIUBuild.pro \
    SPBuild_OPTIM/HCAMBuild.pro
