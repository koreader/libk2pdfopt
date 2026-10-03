-- match(.*(clang-tidy).*)

-------------------------------------------------------------------------------
-- Copyright (c) 2021 Marcus Geelnard
--
-- This software is provided 'as-is', without any express or implied warranty.
-- In no event will the authors be held liable for any damages arising from the
-- use of this software.
--
-- Permission is granted to anyone to use this software for any purpose,
-- including commercial applications, and to alter it and redistribute it
-- freely, subject to the following restrictions:
--
--  1. The origin of this software must not be misrepresented; you must not
--     claim that you wrote the original software. If you use this software in
--     a product, an acknowledgment in the product documentation would be
--     appreciated but is not required.
--
--  2. Altered source versions must be plainly marked as such, and must not be
--     misrepresented as being the original software.
--
--  3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- This wrapper caches results for clang-tidy.
-------------------------------------------------------------------------------

require_std("bcache")
require_std("io")
require_std("os")
require_std("string")
require_std("table")

-------------------------------------------------------------------------------
-- Internal helper functions.
-------------------------------------------------------------------------------

local _OPTIONS_WITH_ARGS = {
  ["-p"] = true,
  ["--checks"] = true,
  ["--config"] = true,
  ["--config-file"] = true,
  ["--export-fixes"] = true,
  ["--extra-arg"] = true,
  ["--extra-arg-before"] = true,
  ["--format-style"] = true,
  ["--header-filter"] = true,
  ["--line-filter"] = true,
  ["--store-check-profile"] = true,
  ["--vfsoverlay"] = true,
  ["--warnings-as-errors"] = true,
}

local function parse_arg (args, idx)
  local arg = args[idx]
  local arg2

  if _OPTIONS_WITH_ARGS[arg] then
    arg2 = args[idx+1]
    return idx+2, arg, arg2
  end

  arg2 = arg:match(".*=(.*)")
  if arg2 then
    arg = arg:match("(.*)=.*")
  end

  return idx+1, arg, arg2
end

local function for_each_unresolved_arg (f)
  local i = 2
  while i <= #m_unresolved_args do
    local arg, arg2
    i, arg, arg2 = parse_arg(m_unresolved_args, i)
    f(arg, arg2)
  end
end

local function for_each_arg (f)
  local i = 2
  while i <= #m_args do
    local arg, arg2
    i, arg, arg2 = parse_arg(m_args, i)
    f(arg, arg2)
  end
end

local _SOURCE_FILE_EXTS = {
  [".cpp"] = true,
  [".cc"] = true,
  [".cxx"] = true,
  [".c"] = true,
  [".h"] = true,
}

local function is_source_file (path)
  if path[1] == "-" then
    return false
  end
  local ext = bcache.get_extension(path):lower()
  return _SOURCE_FILE_EXTS[ext]
end

local function is_dir (path)
  local info = bcache.get_file_info(path)
  return info["is_dir"]
end

local function find_file_in_parents (path, name)
  -- Find the bottom directory.
  local dir = path
  if not is_dir(path) then
    dir = bcache.get_dir_part(dir)
  end

  while #dir > 0 do
    -- Check if the file exists at this directory level.
    local file_path = bcache.append_path(dir, name)
    if bcache.file_exists(file_path) then
      return file_path
    end

    -- Up one level...
    dir = bcache.get_dir_part(dir)
  end

  return nil
end

local function load_file (path)
  local f = assert(io.open(path, "rb"))
  local data = f:read("*all")
  f:close()
  return data
end

local function load_config (src_file)
  -- Use the "--config=<string>" argument to find the configuration file.
  local config_path = nil
  for_each_arg (function (arg, arg2)
    if arg == "--config-file" and arg2 then
      config_path = bcache.resolve_path(arg2)
    end
  end)

  -- "When the value is empty, clang-tidy will attempt to find a file named
  -- .clang-tidy for each source file in its parent directories."
  if not config_path then
    config_path = find_file_in_parents(src_file, ".clang-tidy")
  end

  local config = ""
  if config_path then
    config = load_file(config_path)
  end

  return config
end

local function load_compile_db (first_src_file)
  -- Use the "-p=<path>" argument to find compile_commands.json. "For example,
  -- it can be a CMake build directory in which a file named
  -- compile_commands.json exists [or the path of the compile_commands.json
  -- file]."
  local compile_db_path = nil
  for_each_arg (function (arg, arg2)
    if arg == "-p" and arg2 then
      compile_db_path = arg2
    end
  end)
  if compile_db_path and is_dir(compile_db_path) then
    compile_db_path = bcache.append_path(compile_db_path, "compile_commands.json")
  end

  -- "When no build path is specified, a search for compile_commands.json will
  -- be attempted through all parent paths of the first input file."
  if not compile_db_path then
    compile_db_path = find_file_in_parents(first_src_file, "compile_commands.json")
  end

  if (not compile_db_path) or (not bcache.file_exists(compile_db_path)) then
    error("No compile_commands.json file found")
  end
  bcache.log_debug("Found compile_commands.json: " .. compile_db_path)

  -- Load the compile database.
  return bcache.parse_json(load_file(compile_db_path))
end

local function get_common_path_prefix(path1, path2)
  local it1 = bcache.get_dir_part(path1):gmatch("([^/]+)")
  local it2 = bcache.get_dir_part(path2):gmatch("([^/]+)")
  local prefix = {}
  while true do
    local seg1, seg2 = it1(), it2()
    if not seg1 or seg1 ~= seg2 then
      break
    end
    table.insert(prefix, seg1)
  end
  return table.concat(prefix, "/"), #prefix
end

local function get_compile_cmd (compile_db, src_path)
  for _, entry in ipairs(compile_db) do
    if entry.file == src_path then
      return entry.command, entry.directory
    end
  end
  bcache.log_debug("Entry for " .. src_path .. " not found in compilation database")
  local src_base = bcache.get_file_part(src_path, true)
  local src_stem = bcache.get_file_part(src_path, false)
  local src_ext = bcache.get_extension(src_path)
  local best_match = { score=0 }
  for _, entry in ipairs(compile_db) do
    if is_source_file(entry.file) then
      local score = 10 * select(2, get_common_path_prefix(entry.file, src_path))
      if bcache.get_file_part(entry.file, true) == src_base then
        score = score + 3
      elseif bcache.get_file_part(entry.file, false) == src_stem then
        score = score + 2
      elseif bcache.get_extension(entry.file) == src_ext then
        score = score + 1
      end
      if score > best_match.score then
        best_match.score = score
        best_match.entry = entry
      end
    end
  end
  if best_match.score > 0 then
    bcache.log_debug("Falling back to entry for " .. best_match.entry.file)
    return best_match.entry.command, best_match.entry.directory
  end
  error("No match for " .. src_path .. " found in compilation database")
end

local function extract_compiler_flags (compile_args)
  local i = 0
  local next_arg = function()
      i = i + 1
      return i <= #compile_args and compile_args[i] or nil
  end
  local flags = {}
  for arg in next_arg do
    if arg == "-isystem" or arg:match("^[-/][DIU]$") then
      table.insert(flags, "-"..arg:sub(2))
      table.insert(flags, next_arg())
    elseif arg:match('^[-/][DIU].+$') then
      table.insert(flags, "-"..arg:sub(2))
    end
  end
  return flags
end

local _KNOWN_GCC_PREPROCESSORS = {
  "gcc",
  "g++",
  "clang",
}

local _KNOWN_CPP_PREPROCESSORS = {
  "/usr/bin/cpp",
  "/usr/bin/clang-cpp",
}

local function preprocess_src (src_path, cmd, work_dir)
  -- Get all the arguments for the compiler command.
  local compile_args = bcache.split_args(cmd)

  local pp_type = "?"

  -- Check if we can use the compiler as a preprocessor. The advantages of
  -- using the compiler is that we can be pretty sure that it is installed on
  -- the system, and it should be able to compile the source code that we are
  -- pre-processing, and hopefully it will set relevant macros etc.
  local pp_path = bcache.resolve_path(compile_args[1])
  local pp_name = bcache.get_file_part(pp_path:lower())
  for _, name in ipairs(_KNOWN_GCC_PREPROCESSORS) do
    if pp_name:find(name, nil, true) then
      pp_type = "gcc"
      break
    end
  end
  -- TODO(m): Add support for cl.exe, etc?

  if pp_type == "?" then
    -- Try to find a known preprocessor that is installed on the system.
    for _, path in ipairs(_KNOWN_CPP_PREPROCESSORS) do
      -- TODO(m): We should use find_executable() here.
      if bcache.file_exists(path) then
        pp_path = bcache.resolve_path(path)
        pp_type = "cpp"
        break
      end
    end
  end

  if pp_type == "?" then
    error("Could not find a useful preprocessor")
  end
  bcache.log_debug("Using " .. pp_path .. " for " .. pp_type .. "-style preprocessing")

  -- Construct the command line for running the preprocessor.
  local preprocessor_args = extract_compiler_flags(compile_args)
  table.insert(preprocessor_args, 1, pp_path)
  local preprocessed_file = os.tmpname()
  if pp_type == "gcc" then
    table.insert(preprocessor_args, "-CC")
    table.insert(preprocessor_args, "-E")
    table.insert(preprocessor_args, "-P")
    table.insert(preprocessor_args, "-o")
    table.insert(preprocessor_args, preprocessed_file)
    table.insert(preprocessor_args, src_path)
  elseif pp_type == "cpp" then
    table.insert(preprocessor_args, "-CC")
    table.insert(preprocessor_args, src_path)
    table.insert(preprocessor_args, "-o")
    table.insert(preprocessor_args, preprocessed_file)
  end

  -- Run the preprocessor step.
  local result = bcache.run(preprocessor_args, true, work_dir)
  if result.return_code ~= 0 then
    os.remove(preprocessed_file)
    error("Preprocessing command was unsuccessful:\n" .. result.std_err)
  end

  -- Read the preprocessed file.
  local preprocessed_source = load_file(preprocessed_file)
  os.remove(preprocessed_file)

  -- Include the preprocessor command in the result (different preprocessors
  -- may produce different results).
  return pp_path .. "#:#" .. preprocessed_source
end

-------------------------------------------------------------------------------
-- Wrapper interface implementation.
-------------------------------------------------------------------------------

function can_handle_command ()
  -- Bail for unsupported arguments.
  -- TODO(m):
  --   * Add caching of "--store-check-profile=<prefix>"
  for_each_unresolved_arg (function (arg, arg2)
    if (arg == "--fix") or
       (arg == "--fix-errors") or
       ((arg == "--format-style") and (arg2 ~= "none")) or
       (arg == "--vfsoverlay") or
       (arg == "--store-check-profile")
    then
      error("Unsupported argument: " .. arg)
    end
  end)

  -- Otherwise, go for it!
  return true
end

function get_build_files ()
  local build_files = {}
  local found_fixes_file = false
  for_each_arg (function (arg, arg2)
    if arg == "--export-fixes" and arg2 then
      if found_fixes_file then
        error("Only a single --export-fixes file can be specified.")
      end
      build_files["fixes"] = arg2
      found_fixes_file = true
    end
  end)
  return build_files
end

function get_program_id ()
  -- Get the version string for the program.
  local result = bcache.run({m_args[1], "--version"})
  if result.return_code ~= 0 then
    error("Unable to get the program version information string.")
  end

  return m_args[1] .. ":" .. result.std_out
end

function get_relevant_arguments ()
  local filtered_args = {}

  -- The first argument is the compiler binary without the path.
  table.insert(filtered_args, bcache.get_file_part(m_args[1]))
  for_each_arg (function (arg, arg2)
    -- Ignore arguments that are handled implicitly.
    local ignore_arg = ((arg == "--config-file") or
                        (arg == "--export-fixes") or
                        (arg == "-p"))

    if not ignore_arg then
      if arg2 then
        table.insert(filtered_args, arg .. "=" .. arg2)
      else
        table.insert(filtered_args, arg)
      end
    end
  end)

  bcache.log_debug("Filtered args: " .. table.concat(filtered_args, " "))

  return filtered_args
end

function preprocess_source ()
  -- Collect all source files.
  local src_files = {}
  for_each_arg (function (arg, _)
    if is_source_file(arg) then
      table.insert(src_files, bcache.resolve_path(arg))
    end
  end)
  if next(src_files) == nil then
    error("No source files found")
  end

  bcache.log_debug("Source files: " .. table.concat(src_files, ", "))

  -- Load the compile database to get the compiler command.
  local db = load_compile_db(src_files[1])

  -- Preprocess each source file.
  local input_data_items = {}
  for _, src_path in ipairs(src_files) do
    -- Get the configuration for this file. Inlcude it as it affects the
    -- results of clang-tidy.
    local config = load_config(src_path)
    table.insert(input_data_items, config)

    -- Get the compilation command for this source file.
    local cmd, work_dir = get_compile_cmd(db, src_path)

    -- Preprocess the source.
    table.insert(input_data_items, preprocess_src(src_path, cmd, work_dir))
  end

  -- Return the concatenation of all input data items.
  return table.concat(input_data_items, "#:#")
end

-- vim: sw=2
