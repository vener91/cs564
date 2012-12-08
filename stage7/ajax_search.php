<?php
include("db.php");
$sql = "SELECT song.name, artist.name as artist, album.name as album FROM song, artist, album WHERE song.artist_id = artist.id AND song.album_id = album.id AND (song.name ILIKE :searchTerm OR artist.name ILIKE :searchTerm OR album.name ILIKE :searchTerm) ORDER BY artist.name";
$songRetval = $db->prepare($sql);
$songRetval->execute(array(":searchTerm" => '%' . $_REQUEST['query'] . '%'));
while (($row = $songRetval->fetch(PDO::FETCH_ASSOC))) {
    echo "<tr><td>{$row['name']}<td>{$row['artist']}<td>{$row['album']}";
}
?>
