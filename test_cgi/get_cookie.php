<?php

if(!isset($_COOKIE["name"])) {
    echo "Cookie named 'user' is not set!";
} else {
    echo "Cookie 'user' is set!<br>";
    echo "Value is: " . $_COOKIE['name']."<br>";
    echo "Cookie 'role' is set!<br>";
    echo "Value is: " . $_COOKIE['role'];
}
?>