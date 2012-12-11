<?php
include("db.php");
$query = "";
$params = array();
if (!empty($_REQUEST['song'])) {
    $query .= "AND song.name ILIKE :song ";
    $params[':song'] = "%" . $_REQUEST['song'] . "%";
}

if (!empty($_REQUEST['artist'])) {
    $query .= "AND artist.name ILIKE :artist ";
    $params[':artist'] = "%" . $_REQUEST['artist'] . "%";
}

if (!empty($_REQUEST['album'])) {
    $query .= "AND album.name ILIKE :album ";
    $params[':album'] = "%" . $_REQUEST['album'] . "%";
}

$sql = "SELECT artist.*, album.*, artist.name as artist, album.name as album, song.* FROM song, artist, album WHERE song.artist_id = artist.id AND song.album_id = album.id $query ORDER BY artist.name";
$songRetval = $db->prepare($sql);
$songRetval->execute($params);
while (($row = $songRetval->fetch(PDO::FETCH_ASSOC))) {
    echo "<tr><td>{$row['name']}<td>{$row['duration']}<td>{$row['artist']}<td>{$row['real_name']}<td>{$row['album']}";
}
?>
