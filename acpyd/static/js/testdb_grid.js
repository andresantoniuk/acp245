function log_show_row_pdu(id) {
    var grid = jQuery("#testdb-logs-grid");
    var data = grid.getRowData(id);
    var msg;
    if (data.msg_data !== '') {
        msg = eval('(' + data.msg_data + ')');
        show_pdu(msg);
    }
}
function testdb_grid_show_log(id) {
    var grid = jQuery("#testdb-grid");
    var data = grid.getRowData(id);
    jQuery(".testdb-logs-grid").clearGridData();
    jQuery(".testdb-logs-grid").setGridParam({url: 'testdb/logs.json?test_id=' + id, datatype:'json'});
    jQuery(".testdb-logs-grid").trigger("reloadGrid");
    jQuery('.testdb-logs-grid-box').dialog('option', 'title', 'Log of test ' + data.name + ' (' + data.date + ')');
    jQuery('.testdb-logs-grid-box').dialog('open');
}
function testdb_grid_show_log_link(cellvalue, options, rowObject) {
    return '<a class="underline" href="#" onclick="testdb_grid_show_log(' + options.rowId +');">' + cellvalue + '</a>';
}
function testdb_grid_show_kml_link(cellvalue, options, rowobject) {
    return '<a class="table-icon" target="_blank" href="testdb/travels.kml?test_id=' + options.rowId +'"><span class="ui-icon ui-icon-kml"/></a>';
}
function testdb_grid_show_pdf_link(cellvalue, options, rowobject) {
    return '<a class="table-icon" onclick="pdf_rep_gen('+ options.rowId +');"><span class="ui-icon ui-icon-pdf"/></a>';
}
function pdf_rep_gen (id) {
    jQuery('.pbar-text').text('0%');
    jQuery('.pbar').progressbar("value", 0);
    jQuery('.dlg').dialog('open');
    Nevow.Athena.Widget.get($('.pbar').get(0)).pdf_report_generate(id);
}
function log_show_msg_link(cellvalue, options, rowObject) {
    if (rowObject[3]) {
        return '<a class="underline" href="#" onclick="log_show_row_pdu(' + options.rowId +');">' + cellvalue + '</a>';
    } else {
        return cellvalue;
    }
}
function testdb_grid_init() {
    jQuery(document).ready(function(){
    
    //archive tab's click should refresh the grid
    $('#btn_tab-archive').click(function(){
        $('#refresh_testdb-grid').click();
    });
    
    jQuery("#testdb-grid")
        .jqGrid({
            height: 300,
            width: 600,
            url:'testdb/list.json',
            editurl:'testdb/edit.json',
            altRows: true,
            datatype:'json',
            multiselect: true,
            colNames: [
                'Date',
                'Name',
                'KML',
                'PDF'
            ],
            colModel: [
            {name:'date', width: 200, sortable: false, resizable:false,jsonmap: "cell[0]",
                searchoptions: {sopt:['eq','gt','lt'],
                    dataInit: function(el) {
                        jQuery(el).datepicker({dateFormat:"mm/dd/y", showAnim: 'slideDown'});
                    }
                }
            },
            {name:'name', sortable: false, resizable:false, jsonmap: "cell[1]",
                searchoptions: {sopt:['eq','cn'] },
                formatter: testdb_grid_show_log_link
            },
            {name:'kml', width: 35, align: "center", sortable: false, resizable:false, jsonmap: "cell[0]",
                formatter: testdb_grid_show_kml_link
            },
            {name:'pdf', width: 35, align: "center", sortable: false, resizable:false, jsonmap: "cell[0]",
                formatter: testdb_grid_show_pdf_link
            }
            ],
            jsonReader: {
                repeatitems: false
            },
            pager: jQuery('#testdb-pager'),
            rowNum: 30,
            rowList: [10,30,50,100,200],
            imgpath: '/css/images',
            loadtext: 'Loading...',
            sortname: 'Date',
            caption: 'Tests',
            ondblClickRow: testdb_grid_show_log
        })
        .navGrid("#testdb-pager", {
            refresh: true,
            add: false,
            del: true,
            edit: false,
            search: true
            })
        .navButtonAdd("#testdb-pager", {
            caption: "",
            buttonicon:"ui-icon-pdf",
            onClickButton: function(id) {
                selected = jQuery("#testdb-grid").getGridParam('selarrrow');
                if (selected && selected.length > 0) {
                    window.open('testdb/test.pdf?test_id=' + selected.join(','), '_blank');
                }
                return false;
            },
            position: "last",
            title:"Export to PDF",
            cursor: "pointer"
        });

    jQuery("#testdb-logs-grid")
        .jqGrid({
            height: 300,
            autowidth: true,
            altRows: true,
            userData: [],
            url:'testdb/logs.json',
            datatype:'json',
            colNames: [
                'Date',
                'Level',
                'Message',
                'Message Data'
            ],
            colModel: [
                {name:'date', width: 120, sortable: false},
                {name:'level', width: 60, sortable: false},
                {name:'msg', width: 510, sortable: false, formatter: log_show_msg_link },
                {name:'msg_data', sortable: false, hidden: true}
            ],
            pager: jQuery('#testdb-logs-pager'),
            rowNum: 30,
            rowList: [10,20,30],
            imgpath: '/css/images',
            loadtext: 'Loading...',
            sortname: 'Date',
            caption: 'Events',
            onSelectRow: function(id) {
                var grid = jQuery(".testdb-logs-grid");
                var data = grid.getRowData(id);
                var msg;
                if (data.msg_data !== '') {
                    msg = eval('(' + data.msg_data + ')');
                    show_pdu(msg);
                }
            }
        })
        .navGrid("#testdb-logs-pager", {
            refresh: false,
            add: false,
            del: false,
            edit: false,
            search: false
        });
    jQuery('#testdb-logs-grid-box')
        .dialog({
            autoOpen: false,
            modal: true,
            position: 'center',
            height: 500,
            width: 750,
            buttons: { "Ok": function() { $(this).dialog("close"); } }
        });
    });
}
