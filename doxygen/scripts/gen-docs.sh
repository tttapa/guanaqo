#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
repodir="$PWD"/../..
set -x
cd "$repodir"

mainbranch="main"
output_folder="${1:-/tmp}"
conf_preset="${2:-conan-relwithdebinfo}"
build_preset="${3:-${conf_preset}}"
mkdir -p "$output_folder"

# Function that builds the doxygen documentation and generates the
# coverage information.
# usage:    run_doxygen_coverage <branch-name> <output-directory>
function run_doxygen_coverage {
    branch="$1"
    if [ "$branch" = "$mainbranch" ]; then
        outdir="$2"
    else
        outdir="$2/$branch"
    fi
    htmldir="Doxygen"
    covdir="$outdir/Coverage"
    tmpdir="$repodir/build/docs"
    # Prepare temporary folders
    mkdir -p "$tmpdir"
    echo '*' > "$tmpdir/.gitignore"
    # Remove the old documentation
    mkdir -p "$repodir/docs" "$outdir/$htmldir" "$covdir"
    rm -rf "$outdir/$htmldir" "$covdir"
    rm -rf "$repodir/docs/Coverage"

    # Tweak some Doxyfile verion numbers and output paths
    cat <<- EOF > "$tmpdir/tmp-Doxyfile"
	@INCLUDE = "$repodir/doxygen/Doxyfile"
	PROJECT_NUMBER = "$branch"
	OUTPUT_DIRECTORY = "$outdir"
	HTML_OUTPUT = "$htmldir"
	GENERATE_LATEX = NO
	GENERATE_TAGFILE = "$outdir/$htmldir/guanaqo.tag"
	EOF

    # Configure the project
    cmake --fresh --preset "$conf_preset" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=On \
        -DGUANAQO_WITH_COVERAGE=On \
        -DGUANAQO_FORCE_TEST_DISCOVERY=On \
        -DGUANAQO_DOXYFILE="$tmpdir/tmp-Doxyfile"

    # Generate the Doxygen C++ documentation
    cmake --build --preset "$build_preset" -t docs

    # Generate coverage report
    cmake --build --preset "$build_preset" -t coverage
    mv docs/Coverage "$covdir"

    # Cleanup
    rm -f "$tmpdir/tmp-Doxyfile"
}

# Generate the documentation for the current branch
git fetch ||:
curr_branch=$(git branch --show-current)
if [ -n "$curr_branch" ]; then
    run_doxygen_coverage "$curr_branch" "$output_folder"
elif [ -n "$CI_COMMIT_BRANCH" ]; then
    run_doxygen_coverage "$CI_COMMIT_BRANCH" "$output_folder"
fi
# Generate the documentation for the current tag
git fetch --tags ||:
if curr_tag=$(git describe --tags --exact-match); then
    run_doxygen_coverage "$curr_tag" "$output_folder"
fi

set +x

echo "Done generating documentation"
