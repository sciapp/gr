#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

GR_JL_COMMIT_HASH="9bafb1581c095efa3b5b60827219206e24a713c9"

DIFF_IMAGE_DIR_NAME="diff_images"
export GKS_WSTYPE="png"
GR_BUILD_DIR="$(git rev-parse --show-toplevel)/build"  # Assume a CMake build with `-DGR_BUILD_DEMOS=ON`
if [[ -z "${GRM_SNOOP_EXECUTABLE_PATH}" ]]; then
    GRM_SNOOP_EXECUTABLE_PATH="${GR_BUILD_DIR}/grm_test_public_api/grm_test_snoop/snoop"
fi
if [[ -z "${LIBC_RAND_DIRPATH}" ]]; then
    export LIBC_RAND_DIRPATH="${GR_BUILD_DIR}/grm_test_public_api/grm_test_snoop"
fi


check () {
    if [[ ! -f "${GRM_SNOOP_EXECUTABLE_PATH}" ]]; then
        >&2 echo "The GRM snoop executable \"${GRM_SNOOP_EXECUTABLE_PATH}\" does not exist. Run a CMake build with the"
        >&2 echo "\"-DGR_BUILD_DEMOS=ON\" flag."
        return 1
    fi

    return 0
}

prepare () {
    cd "${SCRIPT_DIR}" && \
    rm -rf "${DIFF_IMAGE_DIR_NAME}" || return
    if [[ ! -e "snoop.jl" ]]; then
        curl -fLO "https://raw.githubusercontent.com/jheinen/GR.jl/${GR_JL_COMMIT_HASH}/examples/snoop.jl" && \
        patch -N -i "snoop.jl.patch" || return
    fi
}

create_grm_images () (
    echo  "Create GRM images..." && \
    mkdir -p "${DIFF_IMAGE_DIR_NAME}/grm" && \
    cd "${DIFF_IMAGE_DIR_NAME}/grm" &>/dev/null && \
    "${GRM_SNOOP_EXECUTABLE_PATH}"
)

create_julia_images () (
    echo  "Create GR.jl images..." && \
    mkdir -p "${DIFF_IMAGE_DIR_NAME}/julia" && \
    cd "${DIFF_IMAGE_DIR_NAME}/julia" &>/dev/null && \
    julia "../../snoop.jl"
)

diff_images () {
    echo  "Compare images..." && \
    mkdir -p "${DIFF_IMAGE_DIR_NAME}/diff" && \
    julia - <<-EOF
		using Images

		function transform_filename_to_index(filename::String)
		    digits = filter(isdigit, filename)
		    if digits != ""
		        return parse(Int, digits)
		    else
		        return 1
		    end
		end

		image_filenames_grm =
		    sort(cd(readdir, "${DIFF_IMAGE_DIR_NAME}/grm"), by = transform_filename_to_index)

		images_differ = false
		for image_filename_grm in image_filenames_grm
		    image_grm = load("${DIFF_IMAGE_DIR_NAME}/grm/" * image_filename_grm)
		    image_julia = load("${DIFF_IMAGE_DIR_NAME}/julia/" * image_filename_grm)

		    if width(image_grm) >= width(image_julia)
		        resized_image_grm = image_grm
		        resized_image_julia = imresize(image_julia, size(image_grm))
		    else
		        resized_image_grm = imresize(image_grm, size(image_julia))
		        resized_image_julia = image_julia
		    end

		    diff_image = map(
		        c -> Gray(clamp01nan(floor(255 * reducec(+, 0.0, c) / 4))),
		        resized_image_grm - resized_image_julia,
		    )

		    if !all(map(c -> isapprox(c, 0.0, atol = 1e-4), diff_image))
		        global images_differ = true
		        println(
		            "Image \$(transform_filename_to_index(image_filename_grm)) differs between GRM and Julia!",
		        )
		        save(
		            "${DIFF_IMAGE_DIR_NAME}/diff/" * image_filename_grm,
		            hcat(resized_image_grm, resized_image_julia, diff_image),
		        )
		    end
		end

		exit(images_differ ? 1 : 0)
	EOF
}

main () {
    check && \
    prepare && \
    create_grm_images && \
    create_julia_images && \
    diff_images
}

main "$@"
