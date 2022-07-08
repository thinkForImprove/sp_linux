TEMPLATE = subdirs

# 定制编译顺序
CONFIG += ordered

# 子项目，一起编译时，是按此顺序编译
SUBDIRS += \
    SPBuild/AllDevPort \
    SPBuild/BRMBuild.pro \
    SPBuild_JIANGNAN/CAMBuild.pro \
    SPBuild_OPTIM/FIDCBuild.pro \
    SPBuild/IDCBuild.pro \
    SPBuild_OPTIM/IDXBuild.pro \
    SPBuild/MSRBuild.pro \
    SPBuild/PINBuild.pro \
    SPBuild/PTRBuild.pro
    

