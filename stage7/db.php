<?php
$host = "postgres.cs.wisc.edu";
$dbname ="cs564_f12";
$schema = "krankit";
try {
    $db = new PDO("pgsql:host=$host;dbname=$dbname");
} catch(PDOException $e) {
    echo $e->getMessage();
}
$db->exec("SET search_path TO $schema");
?>
