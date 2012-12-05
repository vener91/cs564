<?php
include('db.php');
include('header.php');
?>
<div class="header"><a href="index.php" class="logo">Krankit</a><span>Start typing and press save</span></div>
<form class="form-horizontal">
    <div class="control-group">
        <label class="control-label" for="inputId">ID</label>
        <div class="controls">
            <input type="text" id="inputId" value="NEW SONG" readonly>
        </div>
    </div>
    <div class="control-group">
        <label class="control-label" for="inputSongName">Song Name</label>
        <div class="controls">
            <input type="text" id="inputSongName">
        </div>
    </div>
    <div class="control-group">
        <label class="control-label" for="inputArtistName">Artist Name</label>
        <div class="controls">
            <input type="text" id="inputArtistName">
        </div>
    </div>
    <div class="control-group">
        <label class="control-label" for="inputRealName">Artist's Real Name</label>
        <div class="controls">
            <input type="text" id="inputRealName">
        </div>
    </div>
    <div class="control-group">
        <label class="control-label" for="inputBirthdate">Artist's Birthdate</label>
        <div class="controls">
            <input type="text" id="inputBirthdate">
        </div>
    </div>
    <div class="control-group">
        <label class="control-label" for="inputAlbumName">Album Name</label>
        <div class="controls">
            <input type="text" id="inputAlbumName">
        </div>
    </div>
    <div class="control-group">
        <label class="control-label" for="inputReleasedDate">Album Released Date</label>
        <div class="controls">
            <input type="text" id="inputReleasedDate">
        </div>
    </div>
    <div class="control-group">
        <label class="control-label" for="inputAlbumName">Album Name</label>
        <div class="controls">
            <input type="text" id="inputAlbumName">
        </div>
    </div>
    <div class="form-actions">
          <button type="submit" class="btn btn-primary">Save Changes</button>
          <button type="submit" class="btn btn-danger">Delete</button>
    </div>
</form>
<?php include('footer.php')?>
