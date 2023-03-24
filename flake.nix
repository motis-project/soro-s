{
  description = "Stochastic Ordering-Based Railway Operations Simulation";

  inputs =  {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-22.05";

    # the llvm in nixpkgs isn't quite ready yet, but with the fixes from this PR
    # it works
    # FIXME(lgcl): change this to unstable once this is available
    # https://github.com/NixOS/nixpkgs/pull/216273
    unstable.url = "github:rrbutani/nixpkgs/fix/llvm-15-libcxx-linker-script-bug";

    nix-filter.url = "github:numtide/nix-filter";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, unstable, nix-filter, flake-utils, ... }:
    flake-utils.lib.eachSystem [ "x86_64-linux" ] (system:
      let
        name = "soro-s";

        lib = nixpkgs.lib;
        pkgs = unstable.legacyPackages.${system}.extend (final: super: {
          # this produces a git package that will replace ssh pulls from github
          # with https pulls. this is done, because pulling via ssh is very
          # uncomfortable inside nix and many, if not all, dependencies
          # (including transitive ones) that pkg pulls are pull via ssh
          httpsGit = let
            gitConfig = super.writeTextFile {
              name = "https-git-config";
              text = ''
              [url "https://github.com/"]
	              insteadOf = git@github.com:
              '';
              destination = "/.gitconfig";
            };
          in super.symlinkJoin {
            name = "https-git";
            inherit (super.git) version;
            paths = [ super.git ];

            gitConfigHome = gitConfig;

            nativeBuildInputs = [ super.makeWrapper ];
            postBuild = ''
            wrapProgram $out/bin/git \
                 --set HOME $gitConfigHome
            '';
          };

          # these are new variants of fetchgit and fetchFromGitHub using
          # httpsGit. the originals are not overwritten so we don't built
          # everything from source
          fetchgit' = super.fetchgit.override { git = final.httpsGit; };
          fetchFromGitHub' = super.fetchFromGitHub.override { fetchgit = final.fetchgit'; };

          # make sure the tooling uses the up-to date toolchain
          # FIXME(lgcl): drop this once llvmPackages is bumped up to llvmPackages_15
          clang-tools = super.clang-tools.overrideAttrs (old: rec {
            unwrapped = final.llvmPackages_15.clang-unwrapped;
            version = lib.getVersion unwrapped;
            clang = final.llvmPackages_15.clang;
            meta = unwrapped.meta // {
              description = "Standalone command line tools for C++ development";
              maintainers = with lib.maintainers; [ patryk27 ];
            };
          });

          # see: https://github.com/NixOS/nixpkgs/issues/214524
          llvmPackages_15 = super.llvmPackages_15.extend (_: _: let
            llvmPkgs = super.llvmPackages_15;
            inherit (llvmPkgs) tools;
            libraries = llvmPkgs.libraries.extend (libFinal: libSuper: {
              # FIXME(lgcl): remove this, once this is resolved:
              # https://github.com/NixOS/nixpkgs/issues/214524#issuecomment-1431745905
              libcxxabi = libSuper.libcxxabi.overrideAttrs (old: {
                postInstall = old.postInstall or "" + ''
                  install -m 644 ../../${old.pname}/include/__cxxabi_config.h "$dev/include"
                '';
              });
            });
          in { inherit tools libraries; } // libraries // tools);
        });

        llvmPkgs = pkgs.llvmPackages_15;

        # nix-package pkg, used both to pre-fetch the pkg dependecies and during
        # the actual cmake build of soro-s
        motisPkg = llvmPkgs.stdenv.mkDerivation {
          name = "pkg";
          version = "0.14";

          src = pkgs.fetchFromGitHub' {
            owner = "motis-project";
            repo = "pkg";
            rev = "6fa6f5a9e723c4e9a13cf874500281b307a8adb7";
            sha256 = "ya1kU1JXyBP985Dypkd4eXNwt/pslzgDEFJLxrsXCdU=";
            fetchSubmodules = true;
          };

          nativeBuildInputs = [ pkgs.cmake ];

          buildInputs = [];

          installPhase = ''
            install -m 555 -D -t $out/bin pkg
          '';
        };

        # prefetchs a set of pkg managed dependencies, as this takes quite a
        # while and they shouldn't change often (and the .pkg file makes it very
        # easy to notice when they do change)
        #
        # args: specs is the .pkg file (as a path) and sha256 the output hash
        buildPkgDeps = specs: sha256: pkgs.runCommand "pkg-deps" ({
          outputHashMode = "recursive";
          outputHashAlgo = "sha256";
          outputHash = sha256;

          PKG_SPEC = specs;

          nativeBuildInputs = [ motisPkg pkgs.httpsGit pkgs.cacert ];
        }) ''
          cp $PKG_SPEC .pkg
          pkg load -h

          find deps -name .git -type d -prune -exec rm -rf {} \;

          mkdir -p $out
          cp -r deps $out
        '';


        pkgsDir = buildPkgDeps ./.pkg "jcsfB8ssEvD0wxxccpkA+KSU7/XV5WAIUbJ9QJFnzgU=";
        depsDir = pkgs.runCommand "soro-s-deps-dir" {
            PKG_DEPS = pkgsDir;
        } ''
          cp -r "$PKG_DEPS/deps" .
          chmod -R +w deps

          mkdir -p $out
          cp -r deps $out
        '';


        # bypass buildcache (because -DNOBUILDCACHE=1 does not fully work when
        # building the server/client)
        buildcacheStub = pkgs.writeScriptBin "buildcache" ''
          #!/bin/sh
          exec "$@"
        '';
        # disable all pkg calls
        pkgStub = pkgs.writeScriptBin "pkg" ''
          #!/bin/sh
          echo "skipping pkg invocation with args:" "$@"
          exit 0
        '';

        soroSrc = nix-filter.lib.filter {
          root = ./.;
          include = [
            "cmake"
            "include"
            "resources"
            "src"
            "test"
            "tools"
            "web"
            ".pkg"
            "CMakeLists.txt"
            "CMakePresets.json"
            "logo.png"
          ];
        };

        build =
          { useClang ? true
          , version ? "0.0.1"
          , preset ? if useClang then "clang-release" else "gcc-release"
          , target ? "soro-server"
          } : let
            stdenv = if useClang then llvmPkgs.libcxxStdenv else pkgs.gcc11Stdenv;
          in stdenv.mkDerivation {
            name = "soro-s";
            inherit version;

            src = soroSrc;

            DEPS_DIR = depsDir;

            nativeBuildInputs = [
              pkgs.cmake pkgs.ninja

              buildcacheStub pkgStub
            ] ++ (if useClang then [
              llvmPkgs.lld
            ] else []);

            buildInputs = [ pkgs.glibc.static ];

            # provide pre-built pkg dependencies
            # provide stub-pkg at download location
            preConfigure = ''
              cp -r $DEPS_DIR/deps .
              chmod -R +w deps
            '';

            buildPhase = ''
              cmake --build ${preset} --target ${target}
            '';

            installPhase = ''
              find ${preset} -maxdepth 1 -executable -type f \! -name pkg \
                  -exec install -m 555 -D -t $out/bin {} \;
              cp -r ${preset}/server_resources $out/bin/
              cp -r ../resources $out/bin/
            '';

            checkPhase = ''
              runHook preCheck
              cmake --build ${preset} --target soro-test
              ./${preset}/soro-test
              runHook postCheck
            '';

            cmakeFlags = [ "--preset" preset ];
          };

        py = nixpkgs.legacyPackages.${system}.python3.withPackages (pp: [ pp.python-lsp-server pp.selenium ]);
      in rec {
        packages = {
          ${name} = packages."${name}-clang";
          "${name}-gcc" = build { useClang = false; };
          "${name}-clang" = build { useClang = true; };
          default = self.packages.${system}.${name};

          inherit motisPkg depsDir;

          inherit llvmPkgs;
          inherit (llvmPkgs) bintools;
        };

        checks = lib.genAttrs
          [ "${name}-gcc" "${name}-clang" ]
          (p: packages.${p}.overrideAttrs (_: { doCheck = true; }));

        devShells = {
          default = pkgs.mkShell {
            packages = [
              pkgs.clang-tools motisPkg llvmPkgs.libcxxClang
              py pkgs.chromedriver pkgs.chromium
            ] ++ packages."${name}-clang".nativeBuildInputs
            ++ packages."${name}-clang".buildInputs
            ++ packages."${name}-gcc".nativeBuildInputs
            ++ packages."${name}-gcc".buildInputs;
          };
        };

        hydraJobs = let
          prefixAttrNames = prefix: lib.mapAttrs' (n: lib.nameValuePair "${prefix}-${n}");
        in lib.foldr (l: r: l // r) {} ([
          (prefixAttrNames "package" packages)
          (prefixAttrNames "check" checks)
          (prefixAttrNames "shell" devShells)
        ]);
      });
}
