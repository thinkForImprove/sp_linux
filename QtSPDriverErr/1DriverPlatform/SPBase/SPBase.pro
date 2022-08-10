TEMPLATE = subdirs

# 定制编译顺序
CONFIG += ordered

# 子项目，一起编译时，是按此顺序编译
SUBDIRS += \
    SPBaseClass\
    SPBaseBCR\
    SPBaseIDC\
    SPBasePIN\
    SPBaseSIU\
    SPBasePTR\
    SPBaseCRS\
    SPBaseCAM\
    #SPBaseIDX\
    SPBaseVDM\
    #SPBaseMSR\
    SPBaseFIG
