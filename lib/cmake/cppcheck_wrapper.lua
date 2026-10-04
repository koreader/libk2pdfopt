-- match(.*(cppcheck).*)

require_std("bcache")
require_std("io")
require_std("os")
require_std("string")
require_std("table")


local function iter_args()
  local i = 1
  return function()
      i = i + 1
      if i > #m_args then
        return
      end
      local arg = m_args[i]
      local opt, val = arg:match("^(--[^=]+)=(.*)$")
      return opt or arg, val
  end
end

local function is_source_file (path)
  if path:sub(1, 1) == "-" then
    return false
  end
  local info = bcache.get_file_info(path)
  return info and not info["is_dir"]
end

local function load_file (path)
  local f = assert(io.open(path, "rb"))
  local data = f:read("*all")
  f:close()
  return data
end

local function load_compile_db (first_src_file)
  -- Use the "--project=<path>" argument to find compile_commands.json.
  local compile_db_path = nil
  for arg, arg2 in iter_args() do
    if arg == "--project" then
      compile_db_path = arg2
    end
  end

  if (not compile_db_path) or (not bcache.file_exists(compile_db_path)) then
    error("No compile_commands.json file found")
  end
  bcache.log_debug("Found compile_commands.json: " .. compile_db_path)

  return bcache.parse_json(load_file(compile_db_path))
end

local function get_compile_cmd (compile_db, src_path)
  for _, entry in ipairs(compile_db) do
    if entry.file == src_path then
      return entry.command, entry.directory
    end
  end
  error("Entry for " .. src_path .. " not found in compilation database")
end

local function extract_compiler_flags (compile_args)
  local i = 0
  local next_arg = function()
      i = i + 1
      return i <= #compile_args and compile_args[i] or nil
  end
  local flags = {}
  for arg in next_arg do
    if arg == "-isystem" or arg:match("^-[DIU]$") then
      table.insert(flags, arg)
      table.insert(flags, next_arg())
    elseif arg:match('^-[DIU].+$') then
      table.insert(flags, arg)
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
  return true
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
  for arg, arg2 in iter_args() do
    -- Ignore arguments that are handled implicitly.
    local ignore_arg = arg == "--file-filter" or arg == "--project"

    if not ignore_arg then
      if arg2 then
        table.insert(filtered_args, arg .. "=" .. arg2)
      else
        table.insert(filtered_args, arg)
      end
    end
  end

  bcache.log_debug("Filtered args: " .. table.concat(filtered_args, " "))

  return filtered_args
end

function preprocess_source ()
  -- Collect all source files.
  local src_files = {}
  for arg, arg2 in iter_args() do
    if arg == "--file-filter" then
      arg = arg2
    end
    if is_source_file(arg) then
      table.insert(src_files, bcache.resolve_path(arg))
    end
  end
  if next(src_files) == nil then
    error("No source files found")
  end

  bcache.log_debug("Source files: " .. table.concat(src_files, ", "))

  -- Load the compile database to get the compiler command.
  local db = load_compile_db(src_files[1])

  -- Preprocess each source file.
  local input_data_items = {}
  for _, src_path in ipairs(src_files) do
    -- Get the compilation command for this source file.
    local cmd, work_dir = get_compile_cmd(db, src_path)

    -- Preprocess the source.
    table.insert(input_data_items, preprocess_src(src_path, cmd, work_dir))
  end

  -- Return the concatenation of all input data items.
  return table.concat(input_data_items, "#:#")
end

function get_relevant_env_vars ()
  return {
    CLIFORCE_COLOR = "CLIFORCE_COLOR",
  }
end

-- vim: sw=2
