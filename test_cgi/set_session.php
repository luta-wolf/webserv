<?php

    session_start();
    $_SESSION["test"] = "test";
    $value = 'value_test';

    setcookie("cookie_test", $value);
?>