<?php
include("db.php");
function fatalError($msg = null) {
    if ($msg == null) {
        $msg = "Error Occured";
    }
    echo json_encode(array('msg' => $msg));
    die();
}

function ensureArtistAlbum(){
    global $db;
    //Check if artist exists
    $artistRetval = $db->prepare("SELECT * FROM artist WHERE artist.name = :artist");
    $artistRetval->execute(array(":artist" => $_REQUEST['artist']));
    if(!($row = $artistRetval->fetch(PDO::FETCH_ASSOC))){
        //Add a new artist
        $artistRetval = $db->prepare("INSERT INTO artist (name, real_name, birthdate) VALUES (:artist, :real_name, :birthdate)");
        $artistRetval->execute(array(":artist" => $_REQUEST['artist'], ":real_name" => $_REQUEST['real_name'], ":birthdate" => $_REQUEST['birthdate']));
        $errorInfo = $artistRetval->errorInfo();
        if($errorInfo[0] != "00000") { fatalError(); }
        $_REQUEST['artist_id'] = $db->lastInsertId('artist_id');
    } else {
        //Update artist
        $artistRetval = $db->prepare("UPDATE artist SET name = :artist, real_name = :real_name, birthdate = :birthdate WHERE id = :id");
        $artistRetval->execute(array(":artist" => $_REQUEST['artist'], ":real_name" => $_REQUEST['real_name'], ":birthdate" => $_REQUEST['birthdate'], ":id" => $row['id']));
        $errorInfo = $artistRetval->errorInfo();
        if($errorInfo[0] != "00000") { fatalError(); }
        $_REQUEST['artist_id'] = $row['id'];
    }

    $albumRetval = $db->prepare("SELECT * FROM album WHERE album.name = :album");
    $albumRetval->execute(array(":album" => $_REQUEST['album']));
    if(!($row = $albumRetval->fetch(PDO::FETCH_ASSOC))){
        //Add a new album
        $albumRetval = $db->prepare("INSERT INTO album (name, artist_id, released_date) VALUES (:album, :artist_id, :released_date)");
        $albumRetval->execute(array(":album" => $_REQUEST['album'], ":artist_id" => $_REQUEST['artist_id'], ":released_date" => $_REQUEST['released_date']));
        $errorInfo = $albumRetval->errorInfo();
        if($errorInfo[0] != "00000") { fatalError(); }
        $_REQUEST['album_id'] = $db->lastInsertId('album_id');
    } else {
        //Update album
        $albumRetval = $db->prepare("UPDATE album SET name = :album, artist_id = :artist_id, released_date = :released_date WHERE id = :id");
        $albumRetval->execute(array(":album" => $_REQUEST['album'], ":artist_id" => $_REQUEST['artist_id'], ":released_date" => $_REQUEST['released_date'], ":id" => $row['id']));
        $errorInfo = $albumRetval->errorInfo();
        if($errorInfo[0] != "00000") { fatalError(); }
        $_REQUEST['album_id'] = $row['id'];
    }
}

switch ($_SERVER['REQUEST_METHOD']) {
    case 'GET':
        if (!is_numeric($_REQUEST['id'])) {
            fatalError();
        }
        $songRetval = $db->prepare("SELECT song.id, song.name, song.duration, artist.name as artist, artist.real_name, artist.birthdate, album.name as album, album.released_date FROM song, artist, album WHERE song.artist_id = artist.id AND song.album_id = album.id AND song.id = :songId ORDER BY artist.name");
        $songRetval->execute(array(":songId" => $_REQUEST['id']));
        $errorInfo = $songRetval->errorInfo();
        if($errorInfo[0] != "00000") { fatalError(); }
        $row = $songRetval->fetch(PDO::FETCH_ASSOC);
        echo json_encode($row);
        break;
    case 'POST':
        if (is_numeric($_REQUEST['id'])) {
            fatalError();
        }

        ensureArtistAlbum();
        //Finally insert song
        $songRetval= $db->prepare("INSERT INTO song (name, duration, artist_id, album_id) VALUES (:name, :duration, :artist_id, :album_id)");
        $songRetval->execute(array(":name" => $_REQUEST['name'], ":duration" => $_REQUEST['duration'], ":artist_id" => $_REQUEST['artist_id'], ":album_id" => $_REQUEST['album_id']));
        $errorInfo = $songRetval->errorInfo();
        if($errorInfo[0] != "00000") { fatalError(); }
        $_REQUEST['id'] = $db->lastInsertId('song_id');

        $songRetval = $db->prepare("SELECT song.id, song.name, song.duration, artist.name as artist, artist.real_name, artist.birthdate, album.name as album, album.released_date FROM song, artist, album WHERE song.artist_id = artist.id AND song.album_id = album.id AND song.id = :songId ORDER BY artist.name");
        $songRetval->execute(array(":songId" => $_REQUEST['id']));
        $errorInfo = $songRetval->errorInfo();
        if($errorInfo[0] != "00000") { fatalError(); }
        $row = $songRetval->fetch(PDO::FETCH_ASSOC);
        echo json_encode($row);
        break;
    case 'PUT':
        if (!is_numeric($_REQUEST['id'])) {
            fatalError();
        }
        ensureArtistAlbum();
        $songRetval = $db->prepare("UPDATE song SET name = :name, duration = :duration, artist_id = :artist_id, album_id = :album_id WHERE id = :id");
        $songRetval->execute(array(":name" => $_REQUEST['name'], ":duration" => $_REQUEST['duration'], ":artist_id" => $_REQUEST['artist_id'], ":album_id" => $_REQUEST['album_id'], ":id" => $_REQUEST['id']));
        $errorInfo = $songRetval->errorInfo();
        if($errorInfo[0] != "00000") { fatalError(); }
        $_REQUEST['id'] = $db->lastInsertId('song_id');

        $songRetval = $db->prepare("SELECT song.id, song.name, song.duration, artist.name as artist, artist.real_name, artist.birthdate, album.name as album, album.released_date FROM song, artist, album WHERE song.artist_id = artist.id AND song.album_id = album.id AND song.id = :songId ORDER BY artist.name");
        $songRetval->execute(array(":songId" => $_REQUEST['id']));
        $errorInfo = $songRetval->errorInfo();
        if($errorInfo[0] != "00000") { fatalError(); }
        $row = $songRetval->fetch(PDO::FETCH_ASSOC);
        echo json_encode($row);
        break;
    case 'DELETE':
        if (!is_numeric($_REQUEST['id'])) {
            fatalError();
        }

        $songRetval = $db->prepare("DELETE FROM song WHERE id = :id");
        $songRetval->execute(array(":id" => $_REQUEST['id']));
        $errorInfo = $songRetval->errorInfo();
        if($errorInfo[0] != "00000") { fatalError(); }
        break;
}
?>
