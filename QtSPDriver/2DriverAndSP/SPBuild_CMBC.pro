TEMPLATE = subdirs

# 定制编译顺序
CONFIG += ordered

# 子项目，一起编译时，是按此顺序编译
SUBDIRS += \
    SPBuild/AllDevPort \
    SPBuild/BCRBuild.pro \
    SPBuild_CMBC/BRMBuild.pro \
    SPBuild_CMBC/CAMBuild.pro \
    SPBuild_CMBC/FIDCBuild.pro \
    SPBuild_CMBC/FIGBuild.pro \
    SPBuild_CMBC/IDCBuild.pro \
    SPBuild/IDXBuild.pro \
    SPBuild/MSRBuild.pro \
    SPBuild_CMBC/PINBuild.pro \
    SPBuild/PTRBuild.pro \
    SPBuild/SIGBuild.pro \
    SPBuild_CMBC/SIUBuild.pro
    
