// import Nevow.Athena
// import Divmod.Runtime

var messages=[];
function show_pdu_at(i) {
    return show_pdu(messages[i]);
}
function show_pdu(pdu) {
    var tree = document.createElement("div");
    tree.setAttribute("class", "msg_tree");
    build_tree(tree, pdu);
    jQuery(tree).treeview({'collapsed':false});
    jQuery(tree).dialog({title:'Message', height: 400});
    return false;
}

function show_error(text) {
    var p = document.createElement("p");
    p.setAttribute("class", "error-msg");
    p.appendChild(document.createTextNode(text));
    jQuery(p).dialog({title:'Error', height: 200});
}

function escapeHTML(text) {
    return text.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}

acpyd.LiveLabel = Nevow.Athena.Widget.subclass('acpyd.LiveLabel');
acpyd.LiveLabel.methods(
        function __init__(self, widgetNode, text, cls) {
            acpyd.LiveLabel.upcall(self, "__init__", widgetNode);
            self.button = jQuery(self.node).find('.' + cls).get(0);
            self.update_label(text, cls);
        },
        function update_label(self, text, cls) {
            self.text = text;
            self.cls = cls;
            self.button.innerHTML = escapeHTML(self.text);
            self.button.setAttribute('class', self.cls);
        }
);

acpyd.ToggleButton = Nevow.Athena.Widget.subclass('acpyd.ToggleButton');
acpyd.ToggleButton.methods(
    function __init__(self, widgetNode, state, text) {
        acpyd.ToggleButton.upcall(self, "__init__", widgetNode);
        self.button = jQuery(self.node).find('.toggle').get(0);
        jQuery(self.button)
            .hover(
                function () {
                    jQuery(this).removeClass('ui-state-default');
                    jQuery(this).addClass('ui-state-hover');
                },
                function () {
                    jQuery(this).addClass('ui-state-default');
                    jQuery(this).removeClass('ui-state-hover');
                }
            );
        self.update_state(state, text);
    },
    function update_state(self, state, text) {
        self.state = state;
        self.text = text;
        self.button.setAttribute('value', text);
    },
    function toggle(self) {
        self.button.disabled = true;
        d = self.callRemote('toggle');
        d.addCallback(
            function(result) {
                self.button.disabled = false;
            }
        );
        d.addErrback(
            function(failure) {
                show_error(failure.error);
                self.button.disabled = false;
            }
        );
        return false;
    }
);

acpyd.TestbenchLog = Nevow.Athena.Widget.subclass('acpyd.TestbenchLog');
acpyd.TestbenchLog.methods(
    function __init__(self, widgetNode) {
        acpyd.TestbenchLog.upcall(self, "__init__", widgetNode);
        self.log_panel = jQuery(self.node).find('.log').get(0);
        self.log_level = jQuery(self.node).find('.log_level_select').get(0);
        self.scroll_lock_icon = jQuery(self.node).find('.log_scroll_lock').get(0);
        self.scroll_lock = false;
        self.log_int = [];
        for(i = 0; i < self.log_level.options.length; i++) {
            self.log_int[self.log_level.options[i].value] = i;
        }
    },
    function scroll_lock_toggle(self) {
        self.scroll_lock = ! self.scroll_lock;
        if (self.scroll_lock) {
            jQuery(self.scroll_lock_icon).addClass('ui-icon-locked');
            jQuery(self.scroll_lock_icon).removeClass('ui-icon-unlocked');
        } else {
            jQuery(self.scroll_lock_icon).addClass('ui-icon-unlocked');
            jQuery(self.scroll_lock_icon).removeClass('ui-icon-locked');
        }
    },
    function clear_log(self) {
        self.log_panel.innerHTML = '';
        messages = [];
        return false;
    },
    function to_int_level(self, level) {
        var i = self.log_int[level];
        if (i === undefined) {
            return 0;
        } else {
            return i;
        }
    },
    function filter(self) {
        var i;
        var cur_level = self.log_level.selectedIndex;
        var childs = self.log_panel.childNodes;
        for (i = 0; i < childs.length; i++) {
            var cls = childs[i].getAttribute('class');
            var p = cls.indexOf('l_');
            if (p != -1) {
                var level = parseInt(cls.charAt(p+2));
                if (level >= cur_level || level == 0) {
                    childs[i].style.display = '';
                } else {
                    childs[i].style.display = 'none';
                }
            }
        }
        return false;
    },
    function is_level_visible(self, level) {
        var i = self.to_int_level(level);
        return (i >= self.log_level.selectedIndex);/* || (i === 0);*/
    },
    function log_many(self, lines) {
        var i;
        var html = '';
        for (i = 0; i < lines.length; i++) {
            var l = lines[i];
            html += self._log(l[0], l[1], l[2], l[3], l[4], l[5], l[6], l[7], l[8], l[9], l[10]);
        }
        self._appendHTML(html, lines.length);
        if (!self.scroll_lock) {
            self.log_panel.scrollTop = self.log_panel.scrollHeight;
        }
    },
    function log(self, ts, role, ev, text, host_name, host_port, peer_name, peer_port, msg, error, level) {
        if (self.is_level_visible(level)) {
            self._appendHTML(self._log(ts, role, ev, text, host_name, host_port, peer_name, peer_port, msg, error, level), 1);
        }
        if (!self.scroll_lock) {
            self.log_panel.scrollTop = self.log_panel.scrollHeight;
        }
    },
    function _appendHTML(self, html, cnt) {
        var el = document.createElement("span");
        el.innerHTML = html;
        while (el.childNodes.length > 0) {
            self.log_panel.appendChild(el.childNodes.item(0));
        }
        el.innerHTML = '';
    },
    function _log(self, ts, role, ev, text, host_name, host_port, peer_name, peer_port, msg, error, level) {
        var style = '';
        var line;
		var ev_class = '';
        if (level == null) {
            level = 'all';
        }
        if (!self.is_level_visible(level)) {
            style = ' style="display:none;" ';
        }
		if (ev) {
			ev_class = 'ev_' + ev.replace(' ', '_');
		}
        line = '<span class="' + role + ' '+ level +' l_' + self.to_int_level(level) + ' ' + ev_class + '" ' + style + '>'
                + self._pad(ts[3],2) + ':' + self._pad(ts[4],2) + ':' + self._pad(ts[5],2) + ',' + self._pad(parseInt(ts[6]/1000),3)
                + ' [' + role + '] '
                + escapeHTML(text);
        if (msg) {
            line += "&nbsp;<a href='#' onclick='show_pdu_at(" + messages.length +");return false;'>[view MSG]</a>"
            messages.push(msg)
        }
        line += "<br/></span>";
        return line;
    },
    function _pad(self, v, len, padding) {
        if (padding === undefined) {
            padding = '0';
        }
        v = '' + v;
        while (v.length < len) {
            v = padding + v;
        }
        return v;
    }
);

acpyd.TestDbBrowser = Nevow.Athena.Widget.subclass('acpyd.TestDbBrowser');
acpyd.TestDbBrowser.methods(
    function __init__(self, widgetNode, page_size, keys, cols, tests) {
        acpyd.TestDbBrowser.upcall(self, "__init__", widgetNode);
    }
);

acpyd.ScriptEditor = Nevow.Athena.Widget.subclass('acpyd.ScriptEditor');
acpyd.ScriptEditor.methods(
    function __init__(self, widgetNode) {
        acpyd.ScriptEditor.upcall(self, "__init__", widgetNode);
        self.viewing = false;
        self.editing = false;
        self.view_button = jQuery(self.node).find('.view_button').get(0);
        self.edit_button = jQuery(self.node).find('.edit_button').get(0);
        self.save_button = jQuery(self.node).find('.save_button').get(0);
        self.save_as_button = jQuery(self.node).find('.save_as_button').get(0);
        self.save_as_name = jQuery(self.node).find('.save_as_name').get(0);
        self.cancel_button = jQuery(self.node).find('.cancel_button').get(0);
        self.source = jQuery(self.node).find('.script_source').get(0);
        self.editor = jQuery(self.node).find('.script_source_edit').get(0);
        self.select = jQuery(self.node).find('select').get(0);
        self.editor_top = 0;
        self.hide_editor();
        self.hide_viewer();

        jQuery('a', self.node)
            .hover(
                function () {
                    jQuery(this).removeClass('ui-state-default');
                    jQuery(this).addClass('ui-state-hover');
                },
                function () {
                    jQuery(this).addClass('ui-state-default');
                    jQuery(this).removeClass('ui-state-hover');
                }
            );
    },
    function disable_ctrl(self) {
        self.view_button.disabled = true;
        self.edit_button.disabled = true;
        self.save_button.disabled = true;
        self.save_as_button.disabled = true;
        self.save_as_name.disabled = true;
        self.cancel_button.disabled = true;
        self.editor.disabled = true;
    },
    function enable_ctrl(self) {
        self.view_button.disabled = false;
        self.edit_button.disabled = false;
        self.save_button.disabled = false;
        self.save_as_button.disabled = false;
        self.save_as_name.disabled = false;
        self.cancel_button.disabled = false;
        self.editor.disabled = false;
    },
    function toggle_viewer(self) {
        if (self.viewing) {
            self.hide_viewer();
            self.viewing = false;
        } else {
            self.view_script();
        }
        return false;
    },
    function show_editor(self) {
        self.select.disabled = true;
        self.save_button.style.display = '';
        self.save_as_button.style.display = '';
        self.save_as_name.style.display = '';
        self.cancel_button.style.display = '';
        self.editor.scrollTop = self.editor_top;
        self.editor.style.display = '';
        jQuery(self.edit_button).addClass('ui-state-active');
        jQuery(self.edit_button).removeClass('ui-state-default');
    },
    function hide_editor(self) {
        self.select.disabled = false;
        self.editor_top = self.editor.scrollTop;
        self.save_button.style.display = 'none';
        self.save_as_button.style.display = 'none';
        self.save_as_name.style.display = 'none';
        self.cancel_button.style.display = 'none';
        self.editor.style.display = 'none';
        jQuery(self.edit_button).removeClass('ui-state-active');
        jQuery(self.edit_button).addClass('ui-state-default');
    },
    function show_viewer(self) {
        self.select.disabled = true;
        self.viewing = true;
        self.source.style.display = '';
        jQuery(self.view_button).addClass('ui-state-active');
        jQuery(self.view_button).removeClass('ui-state-default');
    },
    function hide_viewer(self) {
        self.select.disabled = false;
        self.viewing = false;
        self.source.style.display = 'none';
        jQuery(self.view_button).removeClass('ui-state-active');
        jQuery(self.view_button).addClass('ui-state-default');
    },
    function filter_key(self) {
        var e = window.event;
        if(e !== undefined && e.key == "tab"){
            event.preventDefault();
            self.insert_tab();
        }
    },
    function insert_tab(self) {
        var start = self.editor.selectionStart;
        var end = self.editor.selectionEnd;
        var value = self.editor.value;
        self.editor.value = value.substring(0, start) + "    " + value.substring(end, value.length);
        start+=4;
        self.editor.setSelectionRange(start, start);
    },
    function view_script(self) {
        var d;
        self.disable_ctrl();
        d = self.callRemote("view_script");
        d.addCallback(
            function(source) {
                self.source.innerHTML=source;
                self.hide_editor();
                self.show_viewer();
                self.enable_ctrl();
            }
        );
        d.addErrback(
            function(failure) {
                show_error(failure.error);
                self.enable_ctrl();
            }
        );
        return false;
    },
    function edit_script(self) {
        var d;
        if (self.editing) {
            self.hide_viewer();
            self.show_editor();
        }

        self.disable_ctrl();
        d = self.callRemote("view_script_raw");
        d.addCallback(
            function(source) {
                self.editor.value = source;
                self.hide_viewer();
                self.start_edit();
                self.enable_ctrl();
            }
        );
        d.addErrback(
            function(failure) {
                show_error(failure.error);
                self.enable_ctrl();
            }
        );
        return false;
    },
    function cancel_edit(self) {
        if (!self.editing) {
            return false;
        }
        self.stop_edit();
        return false;
    },
    function save_script(self) {
        var d;
        var source;
        if (!self.editing) {
            return false;
        }

        self.disable_ctrl();
        source = self.editor.value;
        d = self.callRemote("save_script", source);
        d.addCallback(
            function() {
                self.enable_ctrl();
            }
        );
        d.addErrback(
            function(failure) {
                show_error(failure.error);
                self.enable_ctrl();
            }
        );
        return false;
    },
    function save_as_script(self) {
        var d;
        var source;
        var name;
        if (!self.editing) {
            return false;
        }

        self.disable_ctrl();
        name = self.save_as_name.value;
        source = self.editor.value;
        d = self.callRemote("save_as_script", source, self.save_as_name.value);
        d.addCallback(
            function() {
                var option = document.createElement("option");
                option.setAttribute("value", name);
                option.appendChild(document.createTextNode(name));
                self.select.appendChild(option);
                self.select.selectedIndex = self.select.options.length - 1;
                self.enable_ctrl();
            }
        );
        d.addErrback(
            function(failure) {
                show_error(failure.error);
                self.enable_ctrl();
            }
        );
        return false;
    },
    function start_edit(self) {
        self.editing = true;
        self.show_editor();
    },
    function stop_edit(self) {
        self.editing = false;
        self.hide_editor();
    }
);


acpyd.ReportConfig = Nevow.Athena.Widget.subclass('acpyd.ReportConfig');
acpyd.ReportConfig.methods(
    function __init__(self, widgetNode) {
        acpyd.ReportConfig.upcall(self, "__init__", widgetNode);
        self.fields = {};
        var field_names = ['customer', 'auditor', 'tcu', 'server', 'reference'];
        for (var n in field_names){
                field = field_names[n];
                self.fields[field] = self.nodeByAttribute('name', field);
        }
        //page is loaded, send me the values and if they can be edited
        self.callRemote('updateValues');
        self.callRemote('updateReadonly');
    },
    function edit(self, sender){
        self.callRemote('edit', sender.name, sender.value);
    },
    function setValues(self, customer, auditor, server, tcu, reference){
        self.fields['customer'].value = customer;
        self.fields['auditor'].value = auditor;
        self.fields['server'].value = server;
        self.fields['tcu'].value = tcu;
        self.fields['reference'].value = reference;
    },
    function setReadonly(self, readonly){
        //fill empty inputs with this
        var empty = "Empty";
        if(readonly){
                for(field in self.fields){
                        f = self.fields[field];
                        if($.trim(f.value) == ""){
                                f.value = empty;
                                $(f).addClass('emptyInputComment');
                        }
                        f.setAttribute("readonly", "readonly");
                }
       }
       else{
                for(field in self.fields){
                        f = self.fields[field];
                        if($.trim(f.value) == empty){
                                f.value = "";
                                $(f).removeClass('emptyInputComment');
                        }
                        self.fields[field].removeAttribute("readonly");
                }
       }
    }
);

acpyd.LiveSelect = Nevow.Athena.Widget.subclass('acpyd.LiveSelect');
acpyd.LiveSelect.methods(
    function __init__(self, widgetNode) {
        acpyd.LiveSelect.upcall(self, "__init__", widgetNode);
        self.select = jQuery(self.node).find('select').get(0);
    },
    function selected(self) {
        if (self.select.selectedIndex >= 0) {
            self.select.disabled = true;
            d = self.callRemote('selected', self.select.options[self.select.selectedIndex].value);
            d.addCallback(
                function() {
                    self.select.disabled = false;
                }
            );
            d.addErrback(
                function(failure) {
                    show_error(failure.error);
                    self.select.disabled = false;
                }
            );
        }
    }
);

acpyd.LiveInput = Nevow.Athena.Widget.subclass('acpyd.LiveInput');
acpyd.LiveInput.methods(
    function __init__(self, widgetNode) {
        acpyd.LiveInput.upcall(self, "__init__", widgetNode);
        self.input = jQuery(self.node).find('input').get(0);
    },
    function changed(self) {
        self.input.disabled = true;
        d = self.callRemote('changed', self.input.value);
        d.addCallback(
            function() {
                self.input.disabled = false;
            }
        );
        d.addErrback(
            function(failure) {
                show_error(failure.error);
                self.input.disabled = false;
            }
        );
    }
);

/*
From http://radio.javaranch.com/pascarello/2006/08/17/1155837038219.html
*/
var chatscroll = new Object();
    chatscroll.Pane = function(scrollContainer){
    this.bottomThreshold = 20;
    this.scrollContainer = scrollContainer;
    this._lastScrollPosition = 100000000;
}

chatscroll.Pane.prototype.activeScroll = function() {
    var _ref = this;
    var scrollDiv = this.scrollContainer;
    var currentHeight = 0;
    var _getElementHeight = function(){
        var intHt = 0;
        if(scrollDiv.style.pixelHeight)intHt = scrollDiv.style.pixelHeight;
        else intHt = scrollDiv.offsetHeight;
        return parseInt(intHt);
    }

    var _hasUserScrolled = function(){
        if(_ref._lastScrollPosition == scrollDiv.scrollTop || _ref._lastScrollPosition == null){
            return false;
        }
        return true;
    }

    var _scrollIfInZone = function(){
        if( !_hasUserScrolled ||
                (currentHeight - scrollDiv.scrollTop - _getElementHeight() <= _ref.bottomThreshold)){
            scrollDiv.scrollTop = currentHeight;
            _ref._isUserActive = false;
        }
    }
    if (scrollDiv.scrollHeight > 0)currentHeight = scrollDiv.scrollHeight;
    else if(scrollDiv.offsetHeight > 0)currentHeight = scrollDiv.offsetHeight;
    _scrollIfInZone();
    _ref = null;
    scrollDiv = null;
}




acpyd.PdfRepGen = Nevow.Athena.Widget.subclass('acpyd.PdfRepGen');
acpyd.PdfRepGen.methods(
	function __init__(self, widgetNode){
	    acpyd.PdfRepGen.upcall(self, "__init__", widgetNode);
	},
    function pdf_report_finished(self, mode, test_id) {
        jQuery('.pbar').progressbar('value', 100);
        jQuery('.dlg').dialog('close');
        window.open("/"+mode+"/testdb/test.pdf?id="+test_id, 'PDF Report').focus();
	},
    function pdf_report_progressbar_update(self, val){
        if  (jQuery('.dlg').dialog('isOpen') == false){
            jQuery('.dlg').dialog('open');
        }
        jQuery('.pbar-text').text(val+'%');
        jQuery('.pbar').progressbar("value", val);
    },
    function pdf_report_generate(self, test_id){
        d = self.callRemote('testdb_report_pdf_generate', test_id);
    },
    function pdf_report_cancel(self){
        jQuery('.dlg').dialog('close');
        d = self.callRemote('testdb_report_pdf_cancel');
    },
    function pdf_report_cancel_notify(self){
        jQuery('.dlg').dialog('close');
    }
);
