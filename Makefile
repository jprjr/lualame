.PHONY: release github-release

VERSION = $(shell LUA_PATH="./src/?.lua" lua -e 'print(require("lualame.version")._VERSION)')

github-release:
	source $(HOME)/.github-token && github-release release \
	  --user jprjr \
	  --repo lualame \
	  --tag v$(VERSION)
	source $(HOME)/.github-token && github-release upload \
	  --user jprjr \
	  --repo lualame \
	  --tag v$(VERSION) \
	  --name lualame-$(VERSION).tar.gz \
	  --file dist/lualame-$(VERSION).tar.gz
	source $(HOME)/.github-token && github-release upload \
	  --user jprjr \
	  --repo lualame \
	  --tag v$(VERSION) \
	  --name lualame-$(VERSION).tar.xz \
	  --file dist/lualame-$(VERSION).tar.xz

release:
	rm -rf dist/lualame-$(VERSION)
	rm -rf dist/lualame-$(VERSION).tar.gz
	rm -rf dist/lualame-$(VERSION).tar.xz
	mkdir -p dist/lualame-$(VERSION)/csrc
	rsync -a csrc/ dist/lualame-$(VERSION)/csrc/
	rsync -a src/ dist/lualame-$(VERSION)/src/
	rsync -a CMakeLists.txt dist/lualame-$(VERSION)/CMakeLists.txt
	rsync -a LICENSE dist/lualame-$(VERSION)/LICENSE
	rsync -a README.md dist/lualame-$(VERSION)/README.md
	sed 's/dev/$(VERSION)/g' < lualame-dev-1.rockspec > dist/lualame-$(VERSION)/lualame-$(VERSION)-1.rockspec
	cd dist && tar -c -f lualame-$(VERSION).tar lualame-$(VERSION)
	cd dist && gzip -k lualame-$(VERSION).tar
	cd dist && xz lualame-$(VERSION).tar

