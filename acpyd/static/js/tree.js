
function is_array(a)
{
    return (a.constructor === Array || a instanceof Array);
}
function build_tree(el, json)
{
    var v;
    var k;
    var i;
    var found;
    var root = document.createElement("ul");
    jQuery(root).addClass('treeview-black treeview');

    // Add header
    var fields = json['fields'];
    var start_from = 0;

    // Do not display present field.
    if (fields[0][0] == 'present') {
        start_from = 1;
    }

    for (i = start_from; i < fields.length; i++) {
        k = fields[i][0];
        v = fields[i][1];
        if (v === undefined) {
            continue;
        }
        var v_type = typeof v;
        var li = document.createElement("li");
        var bullet = document.createElement('span');
        li.appendChild(bullet);
        if (v === null) {
            continue;
        } else if (v_type == 'string' || v_type == 'number') {
            li.appendChild(document.createTextNode(k + "=" + v));
        } else if( is_array(v) && (v.length == 0 || typeof(v[0]) != 'object')) {
            var s = '[';
            var v_i = 0;
            for (v_i = 0; v_i < v.length;  v_i++) {
                s = s + v[v_i] + ', ';
            }
            if (s.charAt(s.length-1) === ' ') {
                s = s.substring(0, s.length -2) + "]";
            }
            li.appendChild(document.createTextNode(k + "=" + s));
	} else if( is_array(v)) {
            var v_i = 0;
            var ul = document.createElement("ul");
            for (v_i = 0; v_i < v.length;  v_i++) {
                var sub_li = document.createElement('li');
                var sub_bullet = document.createElement('span');
                sub_li.appendChild(bullet);
                sub_li.appendChild(document.createTextNode('[' + v_i + ']'));
                build_tree(sub_li, v[v_i]);
                ul.appendChild(sub_li);
            }
            li.appendChild(document.createTextNode(k));
            li.appendChild(ul);
	} else {
            // It's an element, must have fields
            var el_fields = v['fields'];
            if (!el_fields) {
                alert(json);
            }

            if (el_fields[0][0] == 'present' && el_fields[0][1] == 0) {
                // first field was present = 0, element is empty.
                li.appendChild(document.createTextNode(k + ' (empty)'));
            } else {
                // first field was present = 1 or no present field,
                // show content
                li.appendChild(document.createTextNode(k));
                build_tree(li, v);
            }
        }
        root.appendChild(li);
    }
    el.appendChild(root);
    return el;
}

