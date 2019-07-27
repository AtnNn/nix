{}:

let nixpkgs = import ../nixpkgs { }; in

let fp = builtins.mapAttrs (name: pkg:
  pkg.overrideAttrs (attrs: {
    meta = attrs.meta // { platforms = attrs.meta.platforms ++ nixpkgs.lib.platforms.all; };
  })); in

with nixpkgs.pkgsCross.mingwW64.extend (self: super: {
  readline = self.readline80;
  db = self.db62;
  bash = self.bash_5;

#  flex = super.flex.override {
#    buildInputs = super.flex.buildInputs ++ [ self.windows.libgnurx ];
#  };
#  bash = self.bash_5;
#  pythonPackages = self.pythonPackages //
#    fp (with super; {inherit pyasn1;});
#} // fp (with super; { inherit readline db mercurial; })
});

with import ./release-common.nix { inherit pkgs; };

stdenv.mkDerivation {
  name = "nix";

  buildInputs = buildDeps; # ++ tarballDeps ++ perlDeps;

  inherit configureFlags;

  enableParallelBuilding = true;

  installFlags = "sysconfdir=$(out)/etc";

  shellHook =
    ''
      export prefix=$(pwd)/inst
      configureFlags+=" --prefix=$prefix"
      PKG_CONFIG_PATH=$prefix/lib/pkgconfig:$PKG_CONFIG_PATH
      PATH=$prefix/bin:$PATH
    '';
}
