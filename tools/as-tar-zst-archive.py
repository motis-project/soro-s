import argparse
import tarfile
import os

desc = 'Accepts multiple paths and a output filename and compresses them into a single outputname.tar.zst archive'
parser = argparse.ArgumentParser(description=desc)

# import zstandard after parser has been created
# since now help info can be printed even without zstandard installed
try:
    import zstandard as zstd
except ModuleNotFoundError:
    print("Please installed the zstandard module via \'pip install zstandard\'") 
    os.sys.exit(1)

parser.add_argument('paths', metavar='paths', type=str, nargs='+', help='paths to a file or folder.')
parser.add_argument('-o', '--output_name', dest='outputname', type=str, help='name for the output archive')
args = parser.parse_args()

if args.outputname is None:
    print("Please specify a name for the output archive.")
    os.sys.exit(1)

if len(args.paths) == 0:
    print("Please specifiy at least one path to a file or folder.")
    os.sys.exit(1)

for path_arg in args.paths:
    if not os.path.exists(path_arg):
        print("There is no file or folder at the path", path_arg)
        print("Please specify a valid path to a file or folder.")
        print("No zstd archive was created.")
        os.sys.exit(1)

with tarfile.open('tmp.tar', 'w') as tar_file:
    for path_arg in args.paths:
        if os.path.isdir(path_arg):
            for root, dirs, files in os.walk(path_arg):
                for file in files:
                    tar_file.add(os.path.join(root, file))

        elif os.path.isfile(path_arg):
            tar_file.add(path_arg)
tar_file.close()

with open('tmp.tar', mode='rb') as tar_file:
    contents = tar_file.read()

    compressor = zstd.ZstdCompressor()
    compressed = compressor.compress(contents)

    with open(args.outputname, mode='wb+') as zst_file:
        zst_file.write(compressed)

os.remove('tmp.tar')
