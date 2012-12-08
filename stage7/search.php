<?php
include('db.php');
include('header.php');
?>
<div class="header"><a href="index.php" class="logo">Krankit</a><span>Advanced search</span></div>
<div class="search-list">
    <input type="text" class="artist" placeholder="Artist"  />
    <input type="text" class="song" placeholder="Song"  />
    <input type="text" class="album" placeholder="Album"  />
    <table class="result-list table table-condensed table-striped table-bordered">
        <thead>
            <tr>
                <th>Song Name</th>
                <th>Duration</th>
                <th>Artist</th>
                <th>Real Name</th>
                <th>Album</th>
            </tr>
        </thead>
        <tbody>
        </tbody>
    </table>
</div>
<script>
var lastXHR = $.ajax();
$("input").on('keyup', function(){
    lastXHR.abort();
    $(".result-list tbody").html("Searching ...");
    lasyXHR = $.get("ajax_advance.php", {
        artist: $("input.artist").val(),
            song: $("input.song").val(),
            album: $("input.album").val()
    }, function(list){
        if (list == '') {
            $('.result-list tbody').html('No results');
        } else {
            $('.result-list tbody').html(list);
        }
    }, 'html')
}).trigger('keyup');
</script>
<?php include('footer.php')?>
