<?php include('header.php')?>
<div class="centerpiece">
    <div class="logo">Krankit<span> MusicDB of the century</span></div>
    <div class="search">
        <input type="text" placeholder="Start typing ... eg. Payphone"  />
        <div class="result-warp">
            <table class="result-list table table-condensed table-striped table-bordered">
                <colgroup>
                    <col style="width: 430px;" />
                    <col style="width: 113px;" />
                    <col style="width: 93px;" />
                </colgroup>
                <thead>
                    <tr>
                        <th>Song Name</th>
                        <th>Artist</th>
                        <th>Album</th>
                    </tr>
                </thead>
                <tbody>
                </tbody>
            </table>
        </div>
    </div>
    <div class="links"><a href="list.php">Browse</a> | <a href="search.php">Advanced Search</a> | <a href="edit.php">Add entries</a></div>
</div>
<script>
var lastXHR = $.ajax();
$(".search > input").on('keyup', function(){
    lastXHR.abort();
    if ($('.search > input').val() == '') {
        $(".result-list tbody").html("Enter something to search");
    } else {
        $(".result-list tbody").html("Searching ...");
        lasyXHR = $.get("ajax_search.php", {query: $(".search > input").val()}, function(list){
            if (list == '') {
                $('.result-list tbody').html('No results');
            } else {
                $('.result-list tbody').html(list);
            }
        }, 'html')
    }
}).on('focus', function(){
    if (!$('.centerpiece').hasClass('expanded')) {
        $(".centerpiece").addClass('expanded').animate({marginTop: 50});
        $(".search").animate({height: $(window).height() - 200});
        $(".result-warp").height($(window).height() - 263).fadeIn();
        $(".search > input").trigger('keyup');
    }
}).on('blur', function(){
    if ($('.search > input').val() == '' && $('.centerpiece').hasClass('expanded')) {
        $(".centerpiece").removeClass('expanded').animate({marginTop: 250});
        $(".search").animate({height: 45});
        $(".result-warp").fadeOut();
    }
});
</script>
<?php include('footer.php')?>
