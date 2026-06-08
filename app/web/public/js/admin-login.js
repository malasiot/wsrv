$(document).ready(function() {

    $('#adminLoginForm').submit(function(e) {
        e.preventDefault();

        let $form = $(this);
        let $btn = $form.find('button[type="submit"]');
        let $spinner = $('#loginSpinner');
        let $alertContainer = $('#loginAlert');

        // Clear out previous state alerts
        $alertContainer.addClass('d-none').text('');
        
        // Front-end sanity check
        let username = $('#username').val().trim();
        let password = $('#password').val().trim();

        if (username === "" || password === "") {
            let missingMsg = $form.data('msg-error-empty');
            $alertContainer.removeClass('d-none').text(missingMsg);
            return;
        }

        // Lock form inputs and button actions
        $btn.prop('disabled', true);
        $spinner.removeClass('d-none');

        // Dispatch credential details to your secure backend validation router
        $.ajax({
            url: '/admin/login',
            type: 'POST',
            data: $form.serialize(),
            dataType: 'json',
            success: function(response) {
                if (response.success) {
                    // Redirect straight to protected admin routing area
                    window.location.href = '/admin/dashboard';
                } else {
                    // Show backend error string if validation fails
                    $alertContainer.removeClass('d-none').text(response.message);
                    $btn.prop('disabled', false);
                    $spinner.addClass('d-none');
                }
            },
            error: function() {
                let generalError = $form.data('msg-error-server');
                $alertContainer.removeClass('d-none').text(generalError);
                $btn.prop('disabled', false);
                $spinner.addClass('d-none');
            }
        });
    });

});
