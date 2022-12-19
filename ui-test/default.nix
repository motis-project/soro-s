{ nix-filter, nixosTest, stdenv, soro-s, python3, chromedriver, chromium }:

let
  system = "x86_64-linux";

  username = "soro-s";

  testScripts = stdenv.mkDerivation {
    name = "soro-s-ui-tests";
    src = nix-filter.lib.filter {
      root = ./.;
      exclude = [ (nix-filter.lib.matchExt "nix") ];
    };

    buildPhase = ''
      echo "#!/usr/bin/env bash" > run-ui-tests.sh
      echo "set -e" >> run-ui-tests.sh
      find . -type f -executable | sed 's#^./#'$out/bin/'#' >> run-ui-tests.sh
      chmod +x run-ui-tests.sh
    '';

    installPhase = ''
      mkdir -p $out/bin
      find . -type f -executable -exec cp {} $out/bin \;
    '';
  };
in nixosTest {
  name = "soros-ui-tests";

  nodes = {
    server = { config, pkgs, ... }: {

      systemd.services.soro-server = {
        enable = true;
        description = "Stochastic Ordering-Based Railway Operations Simulation and Optimization";
        serviceConfig = {
          Type = "simple";
          ExecStartPre = pkgs.writeScript "setup-soro-server.sh" ''
            #!/bin/sh
            for d in {resources,server_resources,profile}; do
              if [ ! -e "/var/lib/soro-server/$d" ]; then
                cp -r "${soro-s}/bin/$d" "/var/lib/soro-server/$d"
                chmod -R ug+w "/var/lib/soro-server/$d"
              fi
            done
          '';
          ExecStart = "${soro-s}/bin/soro-server --port 8080 " +
                      "--resource_dir /var/lib/soro-server/resources " +
                      "--server_resource_dir /var/lib/soro-server/server_resources";

          DynamicUser = true;
          BindReadOnlyPaths = "/nix/store";
          RuntimeDirectory = "soro-server";
          StateDirectory = "soro-server";
          WorkingDirectory = "/var/lib/soro-server";
        };
        after = [ "network.target" ];
        wantedBy = [ "multi-user.target" ];

      };

      environment.systemPackages = [
        (python3.withPackages (pp: [ pp.selenium ]))
        chromedriver chromium testScripts
      ];

      users = {
        mutableUsers = false;
      };
    };
  };

  testScript = ''
    start_all()

    server.wait_for_unit("soro-server.service")
    server.wait_for_open_port(8080)

    server.succeed("run-ui-tests.sh")
  '';
}
