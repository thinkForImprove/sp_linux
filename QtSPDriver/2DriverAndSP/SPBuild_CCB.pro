TEMPLATE = subdirs

# 定制编译顺序
CONFIG += ordered

# 子项目，一起编译时，是按此顺序编译
SUBDIRS += \
    SPBuild/BCRBuild.pro \
    SPBuild/BRMBuild.pro \
    SPBuild/CAMBuild.pro \
    SPBuild_OPTIM/FIDCBuild.pro \
    SPBuild_OPTIM/IDCBuild.pro \
    SPBuild_OPTIM/IDXBuild.pro \
    SPBuild/PINBuild.pro \
    SPBuild/PTRBuild.pro \
    SPBuild_OPTIM/SIUBuild.pro \
    SPBuild/VDMBuild.pro
    
