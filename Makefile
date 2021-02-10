#初期化
CFLAGS=
LDFLAGS=
DEPS=
REL_PATH=../..
include Makefile.common
PLACE=$(shell pwd)

#ソースファイル一覧
SRC_FILES=$(wildcard src/*.c)
OBJ_FILES=$(subst src/,obj/,$(patsubst %.c,%.o,$(SRC_FILES)))

#----------- 固有定義 >>>

#ビルドフラグ
CFLAGS+=-I./include -I./ -D__DEBUG_MAIN__ -g
CFLAGS+= -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux -I./libomxil/include
LDFLAGS+=-lpthread -lrt -L/opt/vc/lib -lGLESv2 -lEGL -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -L./libomxil -lomxil -Wl,-rpath=./libomxil

#TARGETCOPY=no

# 依存ファイル検索
# DEPS+=

# ビルドターゲット
# src/aaa.cなら aaaにしてください
TARG=xc_omxPlay

#----------- 固有定義 <<<

.PHONY: all
all: $(TARG)

lib:
	make -C libomxil clean
	make -C libomxil

#----------- 固有定義 <<<

# Preprocess ごのファイルを生成
%.i: src/%.c $(DEPS)
	@echo "  PREPROCESS $(shell basename $@ .o)"
	@$(CC) -o $@ $< $(CFLAGS)

# オブジェクトファイルを生成
obj/%.o: src/%.c $(DEPS)
	@echo "  BUILD   $(shell basename $@ .o)"
	@mkdir -p obj
	@$(CC) -c -o $@ $< $(CFLAGS)

# 実行ファイルを生成
$(TARG) : $(OBJ_FILES)
	make -C libomxil
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)
	@echo @$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

# cscope/tagファイルを生成するための対象ファイルリスト
cscope.files : $(DEPS_TAGS)
	@-rm cscope* 2>/dev/null ; echo >/dev/null
	@echo $(DEPS_TAGS) > cscope.files
	@sed -i -e 's/ /\n/g' cscope.files

# cscope生成
cscope.out : cscope.files
	@echo "Creating cscope database ($(PLACE))"
	@cscope -b

#tagファイル生成
tags : cscope.files
	@echo "Creating tags ($(PLACE))"
	@ctags -L cscope.files

# クリーン
clean:
	make -C libomxil clean
	@echo "  CLEAN  $(PLACE)"
	@-rm -f obj/* *~ core $(TARG) cscope* tags *.i


# タスクターゲットを宣言
.PHONY: clean

# obj/したのファイルを残す
.PRECIOUS: obj/%.o

echo:
	@echo SRC_FILES $(SRC_FILES)
	@echo OBJ_FILES $(OBJ_FILES)


