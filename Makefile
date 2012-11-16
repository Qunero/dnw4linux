SUBDIR = dnw_src secbulk_driver
APP ?= dnw

all:
	for i in ${SUBDIR} ; do \
		[ ! -d $$i ] || $(MAKE) -C $$i all || exit $$? ; \
	done

install:
	for i in ${SUBDIR} ; do \
		[ ! -d $$i ] || $(MAKE) -C $$i install || exit $$? ; \
	done

clean:
	for i in ${SUBDIR} ; do \
		[ ! -d $$i ] || $(MAKE) -C $$i clean || exit $$? ; \
	done
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions ${APP}
	
