jQuery(document).ready(function(){
    jQuery("ui-state-default").hover(
        function(){ 
            $(this).addClass("ui-state-hover"); 
        },
        function(){ 
            $(this).removeClass("ui-state-hover"); 
        }
    );
    jQuery("body").show();
});

jQuery(window).unload(function() {
    /* workaround for athena on IE. nevow 0.9.32 on IE fails to 
     * close the connection on unload event */
    if(Nevow.Athena.page && !Nevow.Athena.page.pageUnloaded) {
        Nevow.Athena.page.pageUnloaded = true;
        Nevow.Athena.page.deliveryChannel.sendCloseMessage();
    }
});
