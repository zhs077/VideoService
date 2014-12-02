#########################################################################################
# This makefile is for an external application/library in the source tree.
#
# App/Library: log4cplus
# Version: 1.0.4
#########################################################################################
ifndef PRJROOT
    $(error You must first source the environment: "source build-env")
endif

TVER=1.1.3
TARCHIVE=log4cplus-${TVER}.tar.gz
TDISTS=.
TDIR=log4cplus-${TVER}
.PHONY: install clean cleanall
all:
	@echo "uncompress log4cplus-${TVER}.tar.gz ..."
	@if test ! -d ${TDIR};then mkdir -p ${TDIR};\
	tar --touch -xvzf ${TDISTS}/${TARCHIVE};\
	cd ${TDIR} && ./configure --prefix=${EPGIS_INSTALL_PATH};\
	fi
	make -C ${TDIR}

install:
	make -C ${TDIR} install
clean:
	@rm -rfv ${TDIR}

