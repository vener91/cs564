<?php
include('db.php');
include('header.php');
?>
<div class="header"><a href="index.php" class="logo">Krankit</a><span>Start typing and press save</span></div>
<div class="alert edit-alert">
    You have problems
</div>
<form class="form-horizontal">
    <div class="control-group">
        <label class="control-label" for="id">ID</label>
        <div class="controls">
            <input type="text" id="id" value="NEW SONG" readonly>
        </div>
    </div>
    <div class="control-group">
        <label class="control-label" for="name">Song Name</label>
        <div class="controls">
            <input type="text" id="name" placeholder="eg. Turn up the music">
        </div>
    </div>
    <div class="control-group">
        <label class="control-label" for="duration">Duration</label>
        <div class="controls">
            <input type="number" id="duration" placeholder="Duration in seconds">
        </div>
    </div>
    <div class="control-group">
        <label class="control-label" for="artist">Artist Name</label>
        <div class="controls">
            <input type="text" id="artist" placeholder="eg. Chris Brown">
        </div>
    </div>
    <div class="control-group">
        <label class="control-label" for="real_name">Artist's Real Name</label>
        <div class="controls">
            <input type="text" id="real_name" placeholder="eg. Christopher Maurice Brown">
        </div>
    </div>
    <div class="control-group">
        <label class="control-label" for="birthdate">Artist's Birthdate</label>
        <div class="controls">
            <input type="text" id="birthdate" placeholder="eg. 1989-05-05">
        </div>
    </div>
    <div class="control-group">
        <label class="control-label" for="album">Album Name</label>
        <div class="controls">
            <input type="text" id="album" placeholder="Fortune">
        </div>
    </div>
    <div class="control-group">
        <label class="control-label" for="released_date">Album Released Date</label>
        <div class="controls">
            <input type="text" id="released_date" placeholder="eg. 2012-06-29">
        </div>
    </div>
    <div class="form-actions">
          <button type="button" class="btn btn-primary save-btn">Save Changes</button>
          <button type="button" class="btn btn-danger delete-btn">Delete</button>
    </div>
</form>
<script>
$(document).ready(function(){
    if (window.location.hash != '') {
        $.ajax({
            type: "GET",
                dataType: "json",
                url: "ajax_crud.php",
                data: {id: window.location.hash.substr(1)},
                success: function(data){
                    if (typeof(data) === 'object') {
                        if (typeof(data.msg) !== 'undefined') {
                            fatalError(data.msg);
                        } else {
                            $.each(data, function(key, value){
                                $('input#' + key).val(value);
                            });
                            window.location.hash = data.id;
                            success("Song added");
                        }
                    }
                }
        });
    }

    function fatalError(msg){
        $('.edit-alert').addClass('alert-error').removeClass('alert-success').text(msg).show();
    }

    function success(msg){
        $('.edit-alert').addClass('alert-success').removeClass('alert-error').text(msg).show();
    }

    $(".save-btn").on('click', function(){
        $('.edit-alert').hide();
        var submitObj = {};
        $('input').each(function(){
            submitObj[$(this).attr('id')] = $(this).val();
        });
        if ($("input#id").val() === 'NEW SONG') {
            //New item
            $.ajax({
                type: "POST",
                dataType: "json",
                url: "ajax_crud.php",
                data: submitObj,
                success: function(data){
                    if (typeof(data) === 'object') {
                        if (typeof(data.msg) !== 'undefined') {
                            fatalError(data.msg);
                        } else {
                            $.each(data, function(key, value){
                                $('input#' + key).val(value);
                            });
                            window.location.hash = data.id;
                            success("Song added");
                        }
                    }
                }
            });
        } else {
            //Update data
            $.ajax({
                type: "PUT",
                dataType: "json",
                url: "ajax_crud.php",
                data: submitObj,
                success: function(data){
                    if (typeof(data) === 'object') {
                        if (typeof(data.msg) !== 'undefined') {
                            fatalError(data.msg);
                        } else {
                            $.each(data, function(key, value){
                                $('input#' + key).val(value);
                            });
                            window.location.hash = data.id;
                            success("Song updated");
                        }
                    }
                }
            });
        }
        return false;
    });

    $(".delete-btn").on('click', function(){
        $('.edit-alert').hide();
        if ($("input#id").val() === 'NEW SONG') {
            fatalError("");
        } else {
            $.ajax({
                type: "DELETE",
                dataType: "json",
                url: "ajax_crud.php",
                data: {id: $("input#id").val()},
                success: function(data){
                    if (typeof(data) === 'object') {
                        if (typeof(data.msg) !== 'undefined') {
                            fatalError(data.msg);
                        } else {
                            $('input').val("");
                            $("#id").val("NEW SONG");
                            success("Song deleted");
                        }
                    }
                }
            });
        }
        window.location.hash = "";
        return false;
    });
});
</script>
<?php include('footer.php')?>
