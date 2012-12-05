<?php
include('db.php');
$songRetval = $db->query("SELECT song.name, artist.name as artist, album.name as album FROM song, artist, album WHERE song.artist_id = artist.id AND song.album_id = album.id ORDER BY artist.name");
include('header.php');
?>
<div class="header"><a href="index.php" class="logo">Krankit</a><span>Music List</span></div>
<div class="song-list">
    <table class="table table-condensed table-striped table-bordered">
        <thead>
            <tr>
                <th>Song Name</th>
                <th>Artist</th>
                <th>Album</th>
            </tr>
        </thead>
        <tbody>
            <?php
                while (($row = $songRetval->fetch(PDO::FETCH_ASSOC))) {
                    echo "<tr><td>{$row['name']}<td>{$row['artist']}<td>{$row['album']}";
                }
            ?>
        </tbody>
    </table>
</div>
<pre class="debug">
    <?php
//        var_dump(($row = $songRetval->fetch(PDO::FETCH_ASSOC)));
    ?>
</pre>
<?php include('footer.php')?>
