TEMPLATE = subdirs

# 定制编译顺序
CONFIG += ordered

# 子项目，一起编译时，是按此顺序编译
SUBDIRS += \
    SPBuild/AllDevPort \
    SPBuild/BCRBuild.pro \
    SPBuild/BRMBuild.pro \
    SPBuild/CAMBuild.pro \
    SPBuild/CPRBuild.pro \
    SPBuild/CSRBuild.pro \
    SPBuild/FIDCBuild.pro \
    SPBuild/FIGBuild.pro \
    SPBuild/IDCBuild.pro \
    SPBuild/IDXBuild.pro \
    SPBuild/JPRBuild.pro \
    SPBuild/MSRBuild.pro \
    SPBuild/PINBuild.pro \
    SPBuild/PPRBuild.pro \
    SPBuild/PTRBuild.pro \
    SPBuild/SIGBuild.pro \
    SPBuild/SIUBuild.pro \
    SPBuild/VDMBuild.pro
    
