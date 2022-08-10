TEMPLATE = subdirs

# 定制编译顺序
CONFIG += ordered

# 子项目，一起编译时，是按此顺序编译
SUBDIRS += \
    AgentXFS\
    AgentBCR\
    AgentIDC\
    AgentPIN\
    AgentSIU\
    AgentPTR\
    AgentCRS\
    AgentCAM\
    #AgentIDX\
    AgentVDM\
    #AgentMSR\
    AgentFIG
   
