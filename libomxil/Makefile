#初期化
CFLAGS=
LDFLAGS=
DEPS=

# ビルドターゲット
TARG=libomxil.so
all: $(TARG)

#userlandへの相対PATH
REL_PATH=../../
include Makefile.common

#ソースファイル一覧
SRC_FILES=$(wildcard src/*.c)
OBJ_FILES=$(subst src/,obj/,$(patsubst %.c,%.o,$(SRC_FILES)))

#ビルドフラグ
CFLAGS+=-Iinclude -fPIC
CFLAGS+=-DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DHAVE_LIBOPENMAX=2 -DOMX -DOMX_SKIP64BIT -DUSE_EXTERNAL_OMX -DHAVE_LIBBCM_HOST -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM
CFLAGS+=-I./ilclient -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux

# 依存ファイル検索
LIB_DEPS=libpthread librt
# include $(REL_PATH)/common/Makefile.libdeps

# tag/cscopeを使う時の依存ファイル
DEPS_TAGS:=$(DEPS)

install: all
	@echo "  INSTALL $(TARG)"
	@cp -f $(TARG) $(SYSROOT_PATH)/usr/lib
	@cp -rf ./include/* $(SYSROOT_PATH)/usr/include

# Preprocess ごのファイルを生成
%.i: src/%.c $(DEPS)
	@echo "  PREPROCESS $(shell basename $@ .o)"
	@$(CC) -E -o $@ $< $(CFLAGS)

# オブジェクトファイルを生成
obj/%.o: src/%.c $(DEPS)
	@echo "  BUILD   $(shell basename $@ .o)"
	@mkdir -p obj
	@$(CC) -c -o $@ $< $(CFLAGS)

# 実行ファイルを生成
$(TARG) : $(OBJ_FILES)
	@$(CC) -shared $^ -o $@ $(CFLAGS) $(LDFLAGS)
ifeq ($(FW_RELEASE),y)
	@$(STRIP) $@
endif

# cscope/tagファイルを生成するための対象ファイルリスト
cscope.files : $(DEPS_TAGS)
	@-rm cscope* 2>/dev/null ; echo >/dev/null
	@echo $(DEPS_TAGS) > cscope.files
	@sed -i -e 's/ /\n/g' cscope.files

# cscope生成
cscope.out : cscope.files
	@echo "Creating cscope database ($(TARG))"
	@cscope -b

#tagファイル生成
tags : cscope.files
	@echo "Creating tags ($(TARG))"
	@ctags -L cscope.files

# クリーン
clean:
	@echo "  CLEAN  $(TARG)"
	@-rm -f obj/* *~ core $(TARG) cscope* tags *.i *.so *.a
	@-rm -f $(SYSROOT_PATH)/usr/lib/$(TARG)
	@-rm -f $(SYSROOT_PATH)/usr/include/xc_xlp_*.h

# タスクターゲットを宣言
.PHONY: clean
.PHONY: all

# obj/したのファイルを残す
.PRECIOUS: obj/%.o

echo:
	@echo SRC_FILES $(SRC_FILES)
	@echo OBJ_FILES $(OBJ_FILES)


