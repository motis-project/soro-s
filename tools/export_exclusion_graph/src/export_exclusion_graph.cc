#include <fstream>
#include <iostream>

#include "utl/cmd_line_parser.h"
#include "utl/enumerate.h"

#include "soro/infrastructure/infrastructure.h"

using namespace soro::infra;
using namespace utl;

namespace fs = std::filesystem;

struct config {
  cmd_line_flag<std::string, required, UTL_LONG("--infra_path"),
                UTL_DESC("path to the infrastructure directory")>
      infra_path_;

  cmd_line_flag<std::string, required, UTL_LONG("--output_path"),
                UTL_DESC("path to the output file")>
      output_path_;

  bool valid_paths() const {
    return fs::exists(infra_path_.val()) && fs::is_directory(infra_path_.val());
  }
};

int failed_parsing() {
  std::cout << "please set a valid infrastructure directory\n\n";
  std::cout << description<config>();
  std::cout << '\n';

  return 1;
}

void write_out(exclusion_graph const& eg, std::filesystem::path const& out) {
  std::ofstream outfile(out);

  for (auto const [from, node] : utl::enumerate(eg.nodes_)) {
    for (auto const to : node) {
      if (from == to) {
        break;
      }

      outfile << to << ' ';
    }

    outfile << '\n';
  }

  outfile.close();
}

int main(int argc, char const** argv) {
  config c;

  std::cout << "\n\tExclusion Graph Exporter\n\n";
  try {
    c = parse<struct config>(argc, argv);
  } catch (...) {
    return failed_parsing();
  }

  if (!c.valid_paths()) {
    return failed_parsing();
  }

  auto opts = make_infra_opts(c.infra_path_.val(), "");
  opts.layout_ = false;
  opts.interlocking_ = true;
  opts.exclusions_ = true;
  opts.exclusion_graph_ = true;

  infrastructure const infra(opts);

  write_out(infra->exclusion_.exclusion_graph_, c.output_path_.val());

  return 0;
}