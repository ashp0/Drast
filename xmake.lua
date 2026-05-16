set_project("drast")
set_version("0.0.0")

task("test")
    set_category("action")
    set_menu {
        usage = "xmake test",
        description = "Run the Drast compiler test suite, including semantic conformance.",
        options = {}
    }
    on_run(function ()
        local test_script = path.join(os.projectdir(), "tests", "run_tests.sh")
        os.execv("bash", {test_script})
    end)
