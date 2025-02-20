ifeq ($(TARGET),)
  ifeq ($(SRCS),)
    $(error Error: The variables TARGET and SRCS are empty.);
  endif
  USE_SRCS=yes
  TARGET=$(basename $(firstword $(SRCS)))
endif

ifeq ($(OS),Windows_NT)
  ifeq ($(SHELL),cmd)
    CMDSHELL=cmd
  else
    ifeq ($(findstring sh.exe,$(shell where sh.exe 2>&1 | findstr sh.exe)),sh.exe)
      EXT=.sh
    else
      SHELL=cmd
      CMDSHELL=cmd
    endif
  endif
endif

ifeq ($(SCRIPT_DIR),)
  ifeq ($(CMDSHELL),cmd)
    SCRIPT_DIR=..\\..\\
  else
    SCRIPT_DIR=../../
  endif
endif

all: clean
	$(MAKE) run

ifeq (yes,$(USE_SRCS))
$(TARGET): $(SRCS)
  ifeq ($(CMDSHELL),cmd)
	$(SCRIPT_DIR)admb.cmd $(SRCS)
  else
	$(SCRIPT_DIR)admb$(EXT) $(SRCS)
  endif
else
$(TARGET): $(TARGET).tpl $(SRCS)
  ifeq ($(CMDSHELL),cmd)
	$(SCRIPT_DIR)admb.cmd $(TARGET) $(SRCS)
  else
	$(SCRIPT_DIR)admb$(EXT) $(TARGET) $(SRCS)
  endif
endif

ifeq ($(OVERRIDE),all)
_run: $(TARGET)
else
run: $(TARGET)
endif
ifeq ($(CMDSHELL),cmd)
	$(TARGET)
else
	./$(TARGET)
endif

ifeq ($(OVERRIDE),all)
_get-outputs:
else
get-outputs:
endif
ifeq ($(CMDSHELL),cmd)
	@$(SCRIPT_DIR)scripts\\get-outputs.bat $(TARGET)
else
	@sh $(SCRIPT_DIR)scripts/get-outputs.sh $(TARGET)
endif

ifeq ($(OVERRIDE),all)
_clean:
else
clean:
endif
ifeq ($(CMDSHELL),cmd)
	@if exist $(TARGET) del /Q $(TARGET) 2>nul
	@if exist variance del /Q variance 2>nul
	@if exist fmin.log del /Q fmin.log 2>nul
	@if exist $(TARGET).rep del /Q $(TARGET).rep 2>nul
	@if exist $(TARGET).eva del /Q $(TARGET).eva 2>nul
	@if exist $(TARGET).exe del /Q $(TARGET).exe 2>nul
	@if exist $(TARGET).htp del /Q $(TARGET).htp 2>nul
	@if exist $(TARGET).bar del /Q $(TARGET).bar 2>nul
	@if exist $(TARGET).bgs del /Q $(TARGET).bgs 2>nul
	@if exist $(TARGET).cor del /Q $(TARGET).cor 2>nul
ifeq (,$(USE_SRCS))
	@if exist $(TARGET).cpp del /Q $(TARGET).cpp 2>nul
endif
	@if exist $(TARGET).log del /Q $(TARGET).log 2>nul
	@if exist $(TARGET).o del /Q $(TARGET).o 2>nul
	@if exist $(TARGET).obj del /Q $(TARGET).obj 2>nul
	@if exist $(TARGET).par del /Q $(TARGET).par 2>nul
	@if exist $(TARGET).b01 del /Q $(TARGET).b01 2>nul
	@if exist $(TARGET).r01 del /Q $(TARGET).r01 2>nul
	@if exist $(TARGET).p01 del /Q $(TARGET).p01 2>nul
	@if exist $(TARGET).std del /Q $(TARGET).std 2>nul
	@if exist tests_$(TARGET).o del /Q tests_$(TARGET).o 2>nul
	@if exist tests_$(TARGET) del /Q tests_$(TARGET) 2>nul
	@if exist eigv.rpt del /Q eigv.rpt 2>nul
	@if exist admodel.cov del /Q admodel.cov 2>nul
	@if exist admodel.dep del /Q admodel.dep 2>nul
	@if exist admodel.hes del /Q admodel.hes 2>nul
	@if exist $(TARGET).luu del /Q $(TARGET).luu 2>nul
	@if exist $(TARGET).rhes del /Q $(TARGET).rhes 2>nul
	@if exist hesscheck del /Q hesscheck 2>nul
	@if exist hessian.bin del /Q hessian.bin 2>nul
	@if exist posteriors.rep del /Q posteriors.rep 2>nul
	@if exist $(TARGET).ecm del /Q $(TARGET).ecm 2>nul
	@if exist $(TARGET).hst del /Q $(TARGET).hst 2>nul
	@if exist $(TARGET).mc2 del /Q $(TARGET).mc2 2>nul
	@if exist $(TARGET).mcm del /Q $(TARGET).mcm 2>nul
	@if exist $(TARGET).psv del /Q $(TARGET).psv 2>nul
	@if exist $(TARGET).prf  del /Q $(TARGET).prf  2>nul
	@if exist sims del /Q sims 2>nul
else
	@rm -vf $(TARGET)
	@rm -vf admodel.*
	@rm -vf variance
	@rm -vf hessian.bin
	@rm -vf hesscheck
	@rm -vf fmin.log
	@rm -vf $(TARGET).rep
	@rm -vf $(TARGET).eva
	@rm -vf $(TARGET).htp
	@rm -vf $(TARGET).bar
	@rm -vf $(TARGET).bgs
	@rm -vf $(TARGET).cor
ifeq (,$(USE_SRCS))
	@rm -vf $(TARGET).cpp
endif
	@rm -vf $(TARGET).log
	@rm -vf $(TARGET).o
	@rm -vf $(TARGET).obj
	@rm -vf $(TARGET).par
	@rm -vf $(TARGET).p01
	@rm -vf $(TARGET).std
	@rm -vf $(TARGET).luu
	@rm -vf $(TARGET).rhes
	@rm -vf $(TARGET).[bpr]01
	@rm -vf eigv.rpt
	@rm -vf admodel.cov
	@rm -vf admodel.dep
	@rm -vf admodel.hes
	@rm -vf $(TARGET).luu
	@rm -vf $(TARGET).rhes
	@rm -vf hesscheck
	@rm -vf hessian.bin
	@rm -vf posteriors.rep
	@rm -vf $(TARGET).ecm
	@rm -vf $(TARGET).hst
	@rm -vf $(TARGET).mc2
	@rm -vf $(TARGET).mcm
	@rm -vf $(TARGET).psv
	@rm -vf $(TARGET).prf
	@rm -vf sims
endif
