
function gateway_run_command(path) {
    window.frames.result.location = path + '?' + document.getElementById('arguments').value;
    return false;
}

